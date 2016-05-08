use rustbox::{RustBox, Key};

pub enum WidgetRet {
    /// Key is handled by the widget.
    KeyHandled,

    /// Key is ignored by the widget.
    KeyIgnored,

    /// An input is submitted.
    Input(Vec<char>),

    /// Remove the widget. E.g. close the tab, hide the dialogue etc.
    Remove,

    /// An exit event happened.
    Abort,
}

pub trait Widget {
    fn resize(&mut self, width : i32, height : i32);
    fn draw(&self, rustbox : &RustBox, pos_x : i32, pos_y : i32);
    fn keypressed(&mut self, key : Key) -> WidgetRet;
}

// Not sure if this Impl is a good idea -- a stack of widgets is a widget.
impl Widget for Vec<Box<Widget>> {
    fn resize(&mut self, width : i32, height : i32) {
        for widget in self {
            widget.resize(width, height);
        }
    }

    fn draw(&self, rustbox : &RustBox, pos_x : i32, pos_y : i32) {
        for widget in self {
            widget.draw(rustbox, pos_x, pos_y);
        }
    }

    fn keypressed(&mut self, key : Key) -> WidgetRet {
        let ws = self.len();
        self[ws - 1].keypressed(key)
    }
}
