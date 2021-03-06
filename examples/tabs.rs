// Open a lot of tabs. 10 servers tabs, each one having 3 channels.

extern crate ev_loop;
extern crate libc;
extern crate term_input;
extern crate termbox_simple;
extern crate tiny;

use ev_loop::{EvLoop, READ_EV};
use term_input::{Input, Event};
use tiny::tui::tabbed::MsgSource;
use tiny::tui::{TUI, TUIRet, MsgTarget, Timestamp};
use tiny::tui::tabbed::TabStyle;

fn main() {
    let mut tui = TUI::new();

    for serv_idx in 0 .. 10 {
        let server = format!("server_{}", serv_idx);
        tui.new_server_tab(&server, false);

        tui.new_chan_tab(&server, "chan_0", false);
        tui.set_tab_style(TabStyle::NewMsg, &MsgTarget::Chan {
            serv_name: &server,
            chan_name: "chan_0"
        });

        tui.new_chan_tab(&server, "chan_1", false);
        tui.set_tab_style(TabStyle::Highlight, &MsgTarget::Chan {
            serv_name: &server,
            chan_name: "chan_1"
        });

        tui.new_chan_tab(&server, "chan_2", false);
    }

    tui.draw();

    let mut ev_loop: EvLoop<TUI> = EvLoop::new();

    {
        let mut sig_mask: libc::sigset_t = unsafe { std::mem::zeroed() };
        unsafe {
            libc::sigemptyset(&mut sig_mask as *mut libc::sigset_t);
            libc::sigaddset(&mut sig_mask as *mut libc::sigset_t, libc::SIGWINCH);
        };

        ev_loop.add_signal(&sig_mask, Box::new(|_, tui| {
            tui.resize();
            tui.draw();
        }));
    }

    {
        let mut ev_buffer: Vec<Event> = Vec::new();
        let mut input = Input::new();
        ev_loop.add_fd(libc::STDIN_FILENO, READ_EV, Box::new(move |_, ctrl, tui| {
            input.read_input_events(&mut ev_buffer);
            for ev in ev_buffer.drain(0..) {
                match tui.handle_input_event(ev) {
                    TUIRet::Input { msg, from } => {
                        let msg_string = msg.iter().cloned().collect::<String>();
                        match from {
                            MsgSource::Chan { serv_name, chan_name } => {
                                tui.add_privmsg(
                                    "me",
                                    &msg_string,
                                    Timestamp::now(),
                                    &MsgTarget::Chan { serv_name: &serv_name, chan_name: &chan_name });
                            }

                            MsgSource::Serv { .. } => {
                                tui.add_client_err_msg(
                                    "Can't send PRIVMSG to a server.",
                                    &MsgTarget::CurrentTab);
                            }

                            _ => {}
                        }
                    }
                    TUIRet::Abort => {
                        ctrl.stop();
                    }
                    _ => {}
                }
            }
            tui.draw();
        }));
    }

    ev_loop.run(tui);
}
