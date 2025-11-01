void ui_draw_border(int x1, int y1, int x2, int y2)
void ui_draw_window(const char* title, int x, int y, int w, int h)
void ui_display_menu(const char** options, int count, int selected)
void ui_clear_region(int x, int y, int w, int h)
/** input */
int ui_get_key(void)
void ui_read_command(char* buffer, int max_len)
int ui_handle_menu_input(const char** options, int count)
/** loop */
void stdio_ui_start_shell(void)