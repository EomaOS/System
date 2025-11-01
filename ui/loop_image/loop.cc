/**
 * @brief Implementa um loop contínuo que procura por uma imagem (padrão) na tela.
 * * @param pattern_data O ponteiro para os dados brutos de pixel do padrão (imagem) a ser buscado.
 * @param pattern_width Largura da imagem alvo.
 * @param pattern_height Altura da imagem alvo.
 * @param interval_ms O tempo de pausa entre cada busca para não sobrecarregar a CPU.
 * @return Retorna a coordenada (x * 10000 + y) onde o padrão foi encontrado, ou 0 se não encontrado.
 */
long loop_find_image(const unsigned char* pattern_data, int pattern_width, int pattern_height, unsigned int interval_ms) {
    
    // Obtenha os parâmetros do modo gráfico atual (Ex: 1024x768)
    // Essas variáveis devem ser lidas de um struct de estado do seu driver de vídeo
    const int screen_width = 1024; // Exemplo: 1024 pixels
    const int screen_height = 768; // Exemplo: 768 pixels
    
    // 1. Obter o endereço real do Frame Buffer
    unsigned char* screen_buffer = (unsigned char*)get_framebuffer_address();
    
    if (screen_buffer == nullptr) {
        // Log de erro (via sua vga_print_string)
        return 0; 
    }

    int found_x = -1;
    int found_y = -1;
    
    // Loop de busca contínua
    while (1) { // Loop infinito
        
        // 2. Executar o algoritmo de Template Matching
        // Esta função varre o screen_buffer byte a byte (ou pixel a pixel)
        bool found = scan_for_pattern(
            screen_buffer, screen_width, screen_height,
            pattern_data, pattern_width, pattern_height,
            &found_x, &found_y
        );

        if (found) {
            // Se encontrado, retorna a coordenada codificada e encerra o loop
            return (long)found_x * 10000 + found_y;
        }

        // 3. Pausar e liberar a CPU (essencial)
        kernel_sleep_ms(interval_ms);
        
        // (Opcional: Adicionar um contador ou condição de parada aqui se não for infinito)
    }
}