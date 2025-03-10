# Projeto-Final

# Link do Vídeo
https://youtu.be/fpXcrzqLAdg

# Enunciado
Este projeto implementa um sistema de monitoramento baseado no microcontrolador RP2040 (Raspberry Pi Pico). Utiliza um joystick analógico para detectar movimentações, LEDs RGB para sinalização visual e um display OLED SSD1306 para exibição de mensagens. Em caso de movimentos bruscos, o sistema dispara um alerta visual e exibe uma mensagem de socorro no display.

# Descrição do Projeto
O sistema tem como objetivo capturar os valores do joystick em seus eixos X e Y, analisar se atingiram limites predefinidos e, se necessário, ativar um alerta de acidente. Quando um acidente é detectado, os LEDs RGB acendem simultaneamente e a mensagem "HELP ME" é exibida no display OLED. O sistema também imprime uma notificação na interface serial informando sobre o incidente.

# Objetivos
Monitorar movimentos bruscos através da leitura do joystick via ADC.
Gerar alertas visuais utilizando LEDs RGB e um display OLED.
Detectar acidentes e acionar mensagens visuais e via terminal serial.
Usar comunicação I2C e PWM para controle eficiente dos periféricos.

# Funcionalidade
Inicializa o hardware (GPIOs, ADC, PWM e I2C).
Monitora continuamente os valores do joystick.
Aciona alertas se os valores atingirem os limites estabelecidos.
Exibe mensagens no display OLED e LEDs quando necessário.
Envia alertas para a interface serial caso um acidente seja detectado.
