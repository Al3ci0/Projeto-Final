#include <stdio.h> // Biblioteca padrão para entrada e saída
#include <stdlib.h> // Biblioteca padrão para alocação de memória, conversões, etc.
#include "pico/stdlib.h" // Biblioteca padrão do Raspberry Pi Pico
#include "hardware/adc.h" // Biblioteca para manipulação do ADC (Conversor Analógico-Digital)
#include "hardware/i2c.h" // Biblioteca para comunicação I2C
#include "bibli/ssd1306.h" // Biblioteca para controle do display OLED
#include "hardware/pwm.h" // Biblioteca para controle de PWM (Modulação por Largura de Pulso)
#include "hardware/gpio.h" // Biblioteca para controle dos pinos GPIO

// Definições para comunicação I2C com o display OLED
#define I2C_PORT i2c1      
#define I2C_SDA 14         
#define I2C_SCL 15         
#define ENDERECO 0x3C      

// Definições dos pinos do joystick
#define JOYSTICK_X_PIN 26
#define JOYSTICK_Y_PIN 27

// Definições dos pinos dos LEDs
#define LED_R 13    
#define LED_B 12    
#define LED_G 11    

// Definições do tamanho do display OLED
#define WIDTH 128
#define HEIGHT 64

// Estrutura do display OLED e variáveis de controle
ssd1306_t ssd;
volatile bool acidente_ocorreu = false;
absolute_time_t ultima_mensagem;

// Configuração do display OLED
void setup_display() {
    i2c_init(I2C_PORT, 400 * 1000); // Inicializa I2C com frequência de 400 kHz
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Configura pino SDA como função I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Configura pino SCL como função I2C
    gpio_pull_up(I2C_SDA); // Habilita pull-up no pino SDA
    gpio_pull_up(I2C_SCL); // Habilita pull-up no pino SCL
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ENDERECO, I2C_PORT); // Inicializa o display OLED
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    ssd1306_fill(&ssd, false); // Limpa a tela
    ssd1306_send_data(&ssd);
}

// Configuração dos GPIOs para os LEDs
void setup_gpios() {
    stdio_init_all(); // Inicializa comunicação serial
    gpio_init(LED_B);
    gpio_init(LED_R);
    gpio_init(LED_G);
    gpio_set_dir(LED_B, GPIO_OUT);
    gpio_set_dir(LED_R, GPIO_OUT);
    gpio_set_dir(LED_G, GPIO_OUT);
}

// Configuração dos LEDs como saída PWM
void setup_pwm_led(uint gpio) {
    gpio_set_function(gpio, GPIO_FUNC_PWM); // Configura pino para função PWM
    uint slice = pwm_gpio_to_slice_num(gpio); // Obtém o slice PWM do pino
    pwm_set_clkdiv(slice, 25); // Define divisor de clock do PWM
    pwm_set_wrap(slice, 4096); // Define o valor máximo do PWM
    pwm_set_gpio_level(gpio, 0); // Inicializa o PWM com intensidade 0
    pwm_set_enabled(slice, true); // Habilita PWM
}

// Configura PWM para os LEDs
void setup_pwm() {
    setup_pwm_led(LED_R);
    setup_pwm_led(LED_G);
    setup_pwm_led(LED_B);
}

// Função que simula uma interrupção quando o joystick atinge os limites
void acidente_handler(uint gpio, uint32_t events) {
    static absolute_time_t ultimo_evento = {0};
    absolute_time_t agora = get_absolute_time();
    
    if (absolute_time_diff_us(ultimo_evento, agora) > 500000) { // Evita múltiplas ativações em menos de 500ms
        acidente_ocorreu = true;
        ultimo_evento = agora;
    }
}

// Função principal
int main() {
    setup_display(); // Inicializa display OLED
    setup_gpios(); // Configura GPIOs dos LEDs
    setup_pwm(); // Configura PWM para os LEDs
    adc_init(); // Inicializa o ADC (Conversor Analógico-Digital)
    adc_gpio_init(JOYSTICK_X_PIN); // Configura pino do eixo X do joystick para ADC
    adc_gpio_init(JOYSTICK_Y_PIN); // Configura pino do eixo Y do joystick para ADC
    ultima_mensagem = get_absolute_time(); // Armazena o tempo atual
    
    uint16_t margem = 10; // Define a margem de detecção nos limites do joystick
    
    while (true) {
        adc_select_input(0); // Seleciona entrada do eixo X
        uint16_t adc_value_x = adc_read(); // Lê valor do eixo X
        adc_select_input(1); // Seleciona entrada do eixo Y
        uint16_t adc_value_y = adc_read(); // Lê valor do eixo Y
        
        // Converte os valores do ADC para coordenadas na tela do OLED
        uint8_t valor_x = (adc_value_x * WIDTH) / 4095;
        uint8_t valor_y = (adc_value_y * HEIGHT) / 4095;

        // Verifica se o joystick atingiu os limites
        bool atingiu_margem = (valor_x <= margem || valor_x >= (WIDTH - margem) ||
                               valor_y <= margem || valor_y >= (HEIGHT - margem));
        
        if (atingiu_margem) {
            pwm_set_gpio_level(LED_R, 4095); // Acende LED vermelho
            pwm_set_gpio_level(LED_G, 4095); // Acende LED verde
            pwm_set_gpio_level(LED_B, 4095); // Acende LED azul
            ssd1306_fill(&ssd, false); // Limpa a tela
            int x = (WIDTH - (6 * 8)) / 2; // Centraliza "HELP ME" no display
            int y = (HEIGHT - 8) / 2;
            ssd1306_draw_string(&ssd, "HELP ME", x, y); // Exibe "HELP ME" no display
            
            // Simula interrupção para registrar acidente
            acidente_handler(0, 0);
        } else {
            pwm_set_gpio_level(LED_R, 0); // Apaga LED vermelho
            pwm_set_gpio_level(LED_G, 0); // Apaga LED verde
            pwm_set_gpio_level(LED_B, 0); // Apaga LED azul
            ssd1306_fill(&ssd, false); // Limpa o display
        }
        
        ssd1306_send_data(&ssd); // Atualiza o display OLED
        
        // Se um acidente foi detectado, exibe a mensagem na serial
        if (acidente_ocorreu) {
            absolute_time_t agora = get_absolute_time();
            if (absolute_time_diff_us(ultima_mensagem, agora) > 1000000) { // Aguarda 1 segundo entre mensagens
                printf("ACIDENTE NA LOCALIDADE X\n"); // Imprime mensagem no terminal
                ultima_mensagem = agora;
            }
            acidente_ocorreu = false; // Reseta a flag de acidente
        }
    }
    return 0; 
}