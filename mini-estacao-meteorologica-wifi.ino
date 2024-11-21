#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BMP280 bmp; // I2C

// Substitua com as credenciais da sua rede
const char* ssid     = "nome da rede";
const char* password = "senha da rede";

// Define o número da porta do servidor web como 80
WiFiServer server(80);

// Variável para armazenar o pedido HTTP
String header;

// Tempo atual
unsigned long currentTime = millis();
// Tempo anterior
unsigned long previousTime = 0; 
// Define o tempo limite em milissegundos (exemplo: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() 
{
  Serial.begin(9600);
  while ( !Serial ) delay(100);
  
  unsigned status;
  status = bmp.begin(0x76);
  
  if (!status) 
  {
    Serial.println("Não foi possível encontrar um sensor BMP280 válido, verifique o circuito!");
  }
  
  // Conectar à rede Wi-Fi com o SSID e a senha
  Serial.print("Conectando à rede ... ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  // Exibir o endereço IP local e iniciar o servidor web
  Serial.println("");
  Serial.println("Wi-Fi conectado.");
  Serial.println("Endereço IP: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop()
{
  WiFiClient client = server.available();   // Aguarda clientes conectando

  if (client)  // Se um novo cliente conectar
  {                            
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("Novo Cliente");          // Exibe uma mensagem no monitor serial
    String currentLine = "";                // Cria uma string para armazenar dados recebidos do cliente
    
    while (client.connected() && currentTime - previousTime <= timeoutTime)   // Enquanto o cliente estiver conectado
    {
      currentTime = millis();
      if (client.available()) // Se houver bytes para ler do cliente
      {             
        char c = client.read();             // lê um byte, em seguida
        Serial.write(c);                    // imprime no monitor serial
        header += c;
        if (c == '\n') // Se o byte for um caractere de nova linha
        {                    
          // Se a linha atual estiver em branco, você recebeu dois caracteres de nova linha seguidos.
          // Isso indica o fim do pedido HTTP do cliente, então envia uma resposta:
          if (currentLine.length() == 0) 
          {
            // Cabeçalhos HTTP sempre começam com um código de resposta (ex.: HTTP/1.1 200 OK)
            // e um tipo de conteúdo para o cliente saber o que esperar, seguidos por uma linha em branco:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // Exibir a página HTML
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS para estilizar a tabela 
            client.println("<style>body { text-align: center; font-family: \"Trebuchet MS\", Arial;}");
            client.println("table { border-collapse: collapse; width:35%; margin-left:auto; margin-right:auto; }");
            client.println("th { padding: 12px; background-color: #0043af; color: white; }");
            client.println("tr { border: 1px solid #ddd; padding: 12px; }");
            client.println("tr:hover { background-color: #bcbcbc; }");
            client.println("td { border: none; padding: 12px; }");
            client.println(".sensor { color:white; font-weight: bold; background-color: #bcbcbc; padding: 1px; }");
            
            // Título da página
            client.println("</style></head><body><h1>ESP32 com BMP280</h1>");
            client.println("<table><tr><th>MEDIDAS</th><th>VALOR</th></tr>");
            client.println("<tr><td>Temp. Celsius</td><td><span class=\"sensor\">");
            client.println(bmp.readTemperature());
            client.println(" *C</span></td></tr>");  
            client.println("<tr><td>Pressão</td><td><span class=\"sensor\">");
            client.println(bmp.readPressure() / 100.0F);
            client.println(" hPa</span></td></tr>");
            client.println("<tr><td>Altitude Aprox.</td><td><span class=\"sensor\">");
            client.println(bmp.readAltitude(SEALEVELPRESSURE_HPA));
            client.println(" m</span></td></tr>"); 
            client.println("</body></html>");
            
            // A resposta HTTP termina com outra linha em branco
            client.println();
            // Sai do loop while
            break;
          } 
          else 
          { // Se recebeu uma nova linha, limpa a linha atual
            currentLine = "";
          }
        } 
        else if (c != '\r') 
        {  // Se recebeu algo diferente de um retorno de carro,
          currentLine += c;      // adiciona ao final da linha atual
        }
      }
    }
    // Limpa a variável header
    header = "";
    // Fecha a conexão
    client.stop();
    Serial.println("Cliente desconectado.");
    Serial.println("");
  }
}
