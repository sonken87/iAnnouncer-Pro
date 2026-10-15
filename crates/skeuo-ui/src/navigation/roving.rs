//! Roving keyboard focus shared by tab lists and menus: items register their
//! mounted element (in DOM order) and the container moves focus between
//! them with the arrow keys, skipping disabled items.

use std::rc::Rc;

use dioxus::prelude::*;

#[derive(Clone)]
struct Entry {
    key: String,
    disabled: bool,
    el: Rc<MountedData>,
}

/// A list of focusable items, in the order they were mounted.
#[derive(Clone, Copy, PartialEq)]
pub(crate) struct Roving {
    items: Signal<Vec<Entry>>,
}

impl Roving {
    /// Creates the list; call from a hook (`use_hook(Roving::new)`).
    pub(crate) fn new() -> Self {
        Self {
            items: Signal::new(Vec::new()),
        }
    }

    pub(crate) fn register(mut self, key: &str, disabled: bool, el: Rc<MountedData>) {
        let mut items = self.items.write();
        if let Some(e) = items.iter_mut().find(|e| e.key == key) {
            e.el = el;
            e.disabled = disabled;
        } else {
            items.push(Entry {
                key: key.to_string(),
                disabled,
                el,
            });
        }
    }

    pub(crate) fn unregister(mut self, key: &str) {
        if let Ok(mut items) = self.items.try_write() {
            items.retain(|e| e.key != key);
        }
    }

    /// The enabled item `step` places after (or before, if negative) `key`,
    /// wrapping around. `key: None` starts outside the list.
    pub(crate) fn neighbor(self, key: Option<&str>, step: isize) -> Option<String> {
        let items = self.items.peek();
        let n = items.len() as isize;
        if n == 0 {
            return None;
        }
        let mut i = key
            .and_then(|k| items.iter().position(|e| e.key == k))
            .map(|i| i as isize)
            .unwrap_or(if step > 0 { -1 } else { n });
        for _ in 0..n {
            i = (i + step).rem_euclid(n);
            if !items[i as usize].disabled {
                return Some(items[i as usize].key.clone());
            }
        }
        None
    }

    pub(crate) fn first(self) -> Option<String> {
        self.neighbor(None, 1)
    }

    pub(crate) fn last(self) -> Option<String> {
        self.neighbor(None, -1)
    }

    pub(crate) fn focus(self, key: &str) {
        let el = self
            .items
            .peek()
            .iter()
            .find(|e| e.key == key)
            .map(|e| e.el.clone());
        if let Some(el) = el {
            spawn(async move {
                let _ = el.set_focus(true).await;
            });
        }
    }
}
