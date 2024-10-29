#include <BLEDevice.h>
#include <LinkedList.h>

#define LED_AZUL_SCAN_ATIVO 2
#define QUANTIDADE_ITAGS 5
#define QUANTIDADE_MODA 5
#define SEGUNDOS_SCANNER 1
#define TEMPO_PAUSA 10000
#define METRAGEM_QUADRADO 6

int totalScan;
BLEScan* scanner;
class Dispositivo {
  public:
    int Id;
    LinkedList<int> Rssis;
    int Moda;
    float Distancia;
};
LinkedList<Dispositivo*> dispositivos = LinkedList<Dispositivo*>();
std::__cxx11::string iTags[QUANTIDADE_ITAGS] = {
  "fc:58:fa:b4:60:96",
  "fc:58:fa:b4:64:2b",
  "fc:58:fa:b3:87:4f",
  "fc:58:fa:b7:5a:4a",
  "fc:58:fa:b3:8b:94"
};

void setup()
{    
  BLEDevice::init("");
  scanner = BLEDevice::getScan();
  scanner->setActiveScan(true);
  
  Serial.begin(115200);
  pinMode(LED_AZUL_SCAN_ATIVO, OUTPUT);
}

void loop()
{
  totalScan = 0;
  digitalWrite(LED_AZUL_SCAN_ATIVO, HIGH);
  
  acumularRssis();
  definirValores();
  imprimirResultado();
      
  digitalWrite(LED_AZUL_SCAN_ATIVO, LOW);
  delay(TEMPO_PAUSA);
}

void acumularRssis()
{
  do {       
    totalScan++;
    if (totalScan == 1)
      dispositivos.clear();
    BLEScanResults dispositivosEncontrados = realizarScan();
    atualizarLista(dispositivosEncontrados);                        
  } while (totalScan < QUANTIDADE_MODA);  
}

BLEScanResults realizarScan()
{
  BLEScanResults dispositivosEncontrados = scanner->start(SEGUNDOS_SCANNER);
  scanner->stop();
  return dispositivosEncontrados;
}

void atualizarLista(BLEScanResults dispositivosEncontrados)
{
  for(int i = 0; i < dispositivosEncontrados.getCount(); i++) {
    BLEAdvertisedDevice dispositivo = dispositivosEncontrados.getDevice(i);
    for(int j = 0; j < QUANTIDADE_ITAGS; j++) {
      if(dispositivo.getAddress().toString() == iTags[j]) {

        // DISPOSITIVO É UM ITAG
        
        int numeroItag = j + 1;
        Dispositivo *existente;
        bool existe = false;
        
        for(int k = 0; k < dispositivos.size(); k++) {
          existente = dispositivos.get(k);
          if (numeroItag == existente->Id) {
            existe = true;            
            Dispositivo *atualizaElemento = new Dispositivo();
              atualizaElemento->Id = numeroItag;       
              atualizaElemento->Rssis = existente->Rssis;            
              atualizaElemento->Rssis.add(dispositivo.getRSSI());            
            dispositivos.set(k, atualizaElemento);
            break;
          }
        }
        
        if (!existe) {
          Dispositivo *novoElemento = new Dispositivo();
            novoElemento->Id = numeroItag;
            novoElemento->Rssis = LinkedList<int>();            
            novoElemento->Rssis.add(dispositivo.getRSSI());
          dispositivos.add(novoElemento);
        }
        
        break;
      }
    }
  }
}

void definirValores() {
  for(int i = 0; i < dispositivos.size(); i++) {
    Dispositivo *dispositivo = dispositivos.get(i);
    dispositivo->Moda = definirModa(dispositivo);
    dispositivo->Distancia = definirDistancia(dispositivo->Moda);
    
    /*// Simular distancia
    if (dispositivo->Id == 1)
      dispositivo->Distancia = 2.24;
    else if (dispositivo->Id == 2)
      dispositivo->Distancia = 4.12;
    else if (dispositivo->Id == 3)
      dispositivo->Distancia = 2.24;*/
                   
    dispositivos.set(i, dispositivo);
  }
}

int definirModa(Dispositivo *dispositivo) {
  dispositivo->Rssis.sort(funcaoCompara);  
  int ultimoValor = 0, maiorValor = 0, somaAtual = 0, maiorSoma = 0;
  int qtde = dispositivo->Rssis.size();

  for(int j = 0; j < qtde ; j++) {
    int valor = dispositivo->Rssis.get(j);      
    somaAtual = valor == ultimoValor ? somaAtual + 1 : 1;
    if (somaAtual > maiorSoma) {
      maiorSoma = somaAtual;
      maiorValor = valor;
    }
    ultimoValor = valor;
  }

  return maiorValor;
}

int funcaoCompara(int &a, int &b) {
  return a > b;
}

float definirDistancia(int rssi) {
  float environmentalConstant = 0.12; // 2-4 range
  int oneMeterRssi = -56;
  float expoente = (oneMeterRssi - rssi) / 10.0 * environmentalConstant;
  return pow(10, expoente);
}

void imprimirResultado()
{  
  if (dispositivos.size() <= 0) {      
    Serial.println("--- Nenhuma moda encontrada ---");
    return;
  }

  float dist[3] = {0,0,0};

  for(int i = 0; i < dispositivos.size(); i++) {
    Dispositivo *dispositivo = dispositivos.get(i);
    
    Serial.println(" --- ");
    Serial.println(" iTag: " + (String)dispositivo->Id);
    Serial.print(" RSSI: (" + (String)dispositivo->Rssis.size() + ") ");
    for(int j = 0; j < (dispositivo->Rssis.size() - 1); j++)     
      Serial.print((String)dispositivo->Rssis.get(j) + ",");
    Serial.println((String)dispositivo->Rssis.get(dispositivo->Rssis.size() - 1));    
    Serial.println(" Moda: " + (String)dispositivo->Moda);
    Serial.println(" Dist: " + (String)dispositivo->Distancia);
    
    if (dispositivo->Id == 1)
      dist[0] = dispositivo->Distancia;
    else if (dispositivo->Id == 2)
      dist[1] = dispositivo->Distancia;
    else if (dispositivo->Id == 3)
      dist[2] = dispositivo->Distancia;
  }

  if (dist[0] != 0 && dist[1] != 0 && dist[2] != 0)
  {
    float metade = METRAGEM_QUADRADO / 2;
    float x = ((pow(dist[0], 2)) - (pow(dist[1], 2)) + (pow(METRAGEM_QUADRADO, 2))) / (2.0 * METRAGEM_QUADRADO);    
    float y = (((pow(dist[0], 2)) - (pow(dist[2], 2)) + (pow(metade, 2)) + (pow(metade, 2))) / (2.0 * metade)) - x;
    Serial.println(" ------------ Cord: (" + (String)x + "," + (String)y + ")");
  }
}
