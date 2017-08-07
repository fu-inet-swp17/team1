#include "menu.h"
#include "string.h"

void print_menu(menu_t *menu, int curr_item)
{
  lcd_spi_clear(menu->display);
  lcd_spi_draw_s(menu->display, 80, 40, "MAINMENU", 8);

  for(int i = 0; i < menu->cmd_count; ++i) {
    if (i == curr_item) {
      /* Draw Box Arround Item */
    }

    /* Draw Menu Item */
    lcd_spi_draw_s(menu->display, 80, 40, menu->commands[i].name, strlen(menu->commands[0].name));
  }

  lcd_spi_show(menu->display);
}

void menu_init(menu_t *menu, multiplexer_t *mplexer, lcd_spi_t *display, neopixel_t *led_stripe, motor_controller_t *mcontroller,  const menu_command_t *commands, int cmd_count)
{
  menu->multiplexer = mplexer;
  menu->display = display;
  menu->led_stripe = led_stripe;
  menu->mcontroller = mcontroller;
  menu->commands = commands;
  menu->cmd_count = cmd_count;
}

void menu_run(menu_t *menu)
{
  int curr_item = 0;

  while(1) {
    print_menu(menu, curr_item);

    uint16_t action;

    /* Read button from multiplexer */
    for(action = 0;; action = (action + 1) % 3) {
      if (multiplexer_receive(menu->multiplexer, action) == 1)
        break;
    }

    switch(action) {
      case 0:
        /* Execute Action */
        menu->commands[curr_item].handler();
        break;
      case 1:
        /* Move one up */
        if (curr_item - 1 < 0)
          curr_item = menu->cmd_count - 1;
        else
          curr_item--;

        break;
      case 2:
        /* Move one down */
        curr_item = (curr_item + 1) % menu->cmd_count;
        break;
    }
  }
}
