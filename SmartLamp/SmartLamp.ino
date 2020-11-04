#define BLYNK_PRINT Serial 
#include <BlynkSimpleEsp32.h>

#include <WiFi.h>
#include <WiFiClient.h>

// Definição dos pinos a serem utilizados
#define pinR 21
#define pinG 19
#define pinB 18
#define pinPIR 23
#define pinLDR 33


// Token fornecido pelo Blynk
//Necessário para fazer a conexão entre o app e o esp32
char auth[] = "1Ls1DSHKl44ICfnxN_N-hdjlQvOnHbkH";
 
// Credenciais para usar o Wifi.
// Usar "" para redes sem senha.
char ssid[] = "TP-Link_3D4B";
char pass[] = "26071404";

// Configurações para poder usar o PWM
const int freq = 5000;
const int ledChannel1 = 0;
const int ledChannel2 = 1;
const int ledChannel3 = 2;
const int resolution = 8;

// Variáveis de controle
int on_off = 0;          //Variável controlada pelo aplicativo, liga/desliga
int sensores_on_off = 1; //Variável que define se o funcionamento da lâmpada será de acordo com os sensores
int sensor_change = 0;   //Variável que indica se o sensor de presença mudou seu estado
int movimento = 0;       //Variável que indica se há presença ou não
int luz_on_off = 0;      //Variável que indica se há baixa luminosidade ou não
int horario = 0;         //Variável que indica se lâmpada está dentro do horário de funcionamento
int horario_on_off = 0;  //Variável que define se o funcionamento da lâmpada será de acordo com o horario
int brilho = 255;        //Variável que controla o brilho da lâmpada na cor branca
int r = 255;             //Variável que controla o valor do canal de cor vermelha
int g = 255;             //Variável que controla o valor do canal de cor verde
int b = 255;             //Variável que controla o valor do canal de cor azul

// Timer utilizado para chamar periodicamente algumas funções
BlynkTimer timer;
// Relógio necessário para recuperar a hora do dia
//WidgetRTC rtc;

// Quando o Blynk se conectar ao servidor, executa esse trecho de código.
BLYNK_CONNECTED() {
  // Synchronize time on connection
  //rtc.begin();
  //Sincroniza todas as variáveis virtuais utilizadas no projeto.
  Blynk.syncVirtual(V0, V1, V2, V3, V4, V5);
}

// V0: colorpicker para controlar as cores do LED RGB
BLYNK_WRITE(V0){
  // Pega o valor de cada canal de cor e atribui as variaveis r,g,b
  r = param[0].asInt(); // Valor do canal vermelho
  g = param[1].asInt(); // Valor do canal verde
  b = param[2].asInt(); // Valor do canal azul
  Serial.println("trocou a cor");

  // Se a lâmpada estiver ligada, atualiza a cor
  if(on_off == 1 or (luz_on_off and movimento and sensores_on_off) or (horario and horario_on_off)){
      // Se o usuário voltar para a cor branca,
      //ela deverá acender com o valor de brilho previamente informado
      if(r == 255 and g == 255 and b == 255){
        Blynk.syncVirtual(V4);
      }else{     
        ledcWrite(ledChannel1, r);
        ledcWrite(ledChannel2, g);
        ledcWrite(ledChannel3, b);
        Serial.println("Aplicou a cor");
      }
  }
}


// V1: Botão liga/desliga a lâmpada
BLYNK_WRITE(V1){
  // Pega o valor do estado da lâmpada no aplicativo
  on_off = param.asInt();

  // Se o comando enviado foi de ligar, então os canais de cores recebem os valores r,g,b que regem a cor do LED. 
  if(on_off == 1 or (luz_on_off and movimento and sensores_on_off) or (horario and horario_on_off)){
      ledcWrite(ledChannel1, r);
      ledcWrite(ledChannel2, g);
      ledcWrite(ledChannel3, b);
      // Atualiza o botão na tela do app
      //Blynk.virtualWrite(V1,1);   
      // Se quando a lâmpada ligar, ela estiver na cor branca, ajusta o brilho.
      Blynk.syncVirtual(V4);
      
  // Se não a lâmpada é desligada, atribuindo o valor 0 nos canais
  }else{
      ledcWrite(ledChannel1, 0);
      ledcWrite(ledChannel2, 0);
      ledcWrite(ledChannel3, 0);

  }

}

// V2: Input horário de funcionamento da lâmpada
BLYNK_WRITE(V2){
  horario = param.asInt();

 

  // Se houve alteração no horario, chama a função BLYNK_WRITE(V1) 
  //Blynk.syncVirtual(V1);
}

// V3: Botão liga/desliga horário
BLYNK_WRITE(V3){
  // Pega o valor que decide se o funcionamento irá depender do horário.
  horario_on_off = param.asInt();
  Serial.print("Liga/desliga horario: ");
  Serial.println(horario_on_off);

}

// V4: Input brilho lâmpada
BLYNK_WRITE(V4){
  // Pega o valor do brilho
  brilho = param.asInt();

  // Como escrito no aplicativo, o Slider "Brilho" 
  // só controla o brilho da lâmpada se a mesma estiver com a cor branca,
  // pois o colorpicker RGB já possibilita mudar o brilho das cores, menos a cor branca.
  // Se todos os canais de cores estiverem no seu valor máximo (255), o LED ficará branco (na teoria).
  
  // Se os canais apresentarem o mesmo valor máximo, o valor dos canais irão depender do valor que virá do slider no aplicativo
  if(r == 255 and g == 255 and b == 255){
    if(on_off == 1 or (luz_on_off and movimento and sensores_on_off) or (horario and horario_on_off)){
        ledcWrite(ledChannel1, brilho);
        ledcWrite(ledChannel2, brilho);
        ledcWrite(ledChannel3, brilho);
    }
  }
}

// V5: Botão liga/desliga o funcionamento dos sensores
BLYNK_WRITE(V5){
  
  // Pega o valor que decide se o funcionamento irá depender dos sensores.
  sensores_on_off = param.asInt();
  Serial.print("Sensores: ");
  Serial.println(sensores_on_off);
  // Se houve alteração na habilitação do horário, chama a função BLYNK_WRITE(V1) 
  Blynk.syncVirtual(V1);
  
}

// Função de interrupção que é chamada quando há alteração de presença no ambiente.
void IRAM_ATTR sensor_movimento(){

    Serial.println("Presença: ");
    Serial.println(digitalRead(pinPIR));
    if (movimento == 1){
      movimento = 0;
    }else{
      movimento = 1;
    }
    // Variável para informar que houve uma alteração
    sensor_change = 1;
}

// Função que verifica o nivel de luminosidade.
// Threshold: valor inteiro 1000.
// Limites do LDR: 
// *min: 0
// *max: 4095
void verifica_luz(){  
    Serial.print("Sensor LDR: ");
    // Lê o valor da porta pinLDR (35)
    // onde o sensor LDR está conectado.
    int luminosidade = analogRead(pinLDR);
    
    Serial.println(luminosidade);
    // Se o valor lido do LDR for menor que o limite "1000"
    // a lâmpada é acessa.
    if (luminosidade < 1000){
      
      luz_on_off = 1;
      // Envia um comando para poder atualizar o elemento que controla esta variavel no app
      Blynk.virtualWrite(V1, on_off);
      Serial.println("Liga lâmpada");
    }else{
      
      luz_on_off = 0;
      // Envia um comando para poder atualizar o elemento que controla esta variavel no app
      Blynk.virtualWrite(V1, on_off);
      Serial.println("Desliga lâmpada");
    }  
    
    // Envia um comando ao Blynk para sincronizar os dados do app e do esp32.
    // no caso a variável a ser sincronizada é on_off, que é manipulada por um botão no app
    // e que decide se a lâmpada liga ou desliga.
    Blynk.syncVirtual(V1);
}



void setup()
{
  // Console 
  Serial.begin(115200);

  // Configura os canais que serão usados para controlar as portas com PWM.
  ledcSetup(ledChannel1, freq, resolution);
  ledcSetup(ledChannel2, freq, resolution);
  ledcSetup(ledChannel3, freq, resolution);
  
  // Vincula um canal a uma porta GPIO a ser controlada.
  ledcAttachPin(pinR, ledChannel1);
  ledcAttachPin(pinG, ledChannel2);
  ledcAttachPin(pinB, ledChannel3);
  
  // Definindo a porta em que o sensor PIR está como entrada.
  pinMode(pinPIR, INPUT_PULLUP);
  

  //Conecta o esp32 ao servidor do Blynk.
  Blynk.begin(auth, ssid, pass);
  
  Blynk.virtualWrite(V0, 255,255,255);
  Blynk.virtualWrite(V1, on_off);
  Blynk.virtualWrite(V2, horario);
  Blynk.virtualWrite(V3, horario_on_off);
  Blynk.virtualWrite(V4, brilho);
  Blynk.virtualWrite(V5, sensores_on_off);

  //Atribui uma função a interrupção causada pela mudança na porta pinPIR
  attachInterrupt(digitalPinToInterrupt(pinPIR), sensor_movimento, CHANGE);
   
  // Timer que chama a determinada função a cada 10 segundos.
  timer.setInterval(10000L, verifica_luz);
}

void loop()
{
  // Processa comandos e gerencia a conexão com a nuvem
  Blynk.run();
  // Executa os timers configurados previamente
  timer.run();

  // Se houve alteração na presença, chama a função BLYNK_WRITE(V1) 
  if (sensor_change){
    sensor_change = 0;
    Blynk.syncVirtual(V1);
  }

}
