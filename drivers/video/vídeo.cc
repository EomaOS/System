// Definições de Hardware
#define VGA_MEMORY_ADDRESS 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_CRT_CTRL_REG 0x3D4 // Porta de comando do Controlador CRT
#define VGA_CRT_DATA_REG 0x3D5 // Porta de dados do Controlador CRT

// Tipo para cor/atributo (background | foreground)
typedef unsigned char vga_color_t; 

// Funções para controle do buffer de texto VGA
void vga_clear_screen(vga_color_t color_attr);
void vga_put_char_at(char character, vga_color_t color_attr, int row, int col);
void vga_print_string(const char* str, vga_color_t color_attr);
void vga_scroll_up(int lines, vga_color_t color_attr);

// Funções para controle do cursor de hardware VGA
void vga_enable_cursor(unsigned char cursor_start, unsigned char cursor_end);
void vga_disable_cursor();
void vga_update_cursor_pos(int row, int col);

// Funções auxiliares (internas ao driver)
static int vga_get_cursor_offset(int row, int col);