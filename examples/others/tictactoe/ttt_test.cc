#include "tictactoe.h"
#include <gtkmm/window.h>
#include <gtkmm/application.h>


void
win (TicTacToe *ttt)
{
  g_print ("Yay!\n");
  ttt->clear();
}


int
main (int argc, char *argv[])
{
  Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, "org.gtkmm.example");

  TicTacToe* ttt = manage( new TicTacToe );
  ttt->tictactoe.connect( sigc::bind (sigc::ptr_fun(&win), ttt) );

  Gtk::Window window;
  window.set_title("Tic-Tac-Toe");
  window.set_border_width(10);
  window.add(*ttt);
  window.show_all_children();

  return app->run(window);
}
