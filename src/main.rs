use std::{error::Error, thread, time::Duration};

use sdl3::{self, event::Event, keyboard::Keycode, pixels::Color, rect::Rect};

fn main() -> Result<(), Box<dyn Error>> {
	let sdl_context = sdl3::init()?;
	let video_subsystem = sdl_context.video()?;

	let window = video_subsystem
		.window("🦀 Rusteroids 🦀", 800, 600)
		.position_centered()
		.opengl()
		.build()?;

	let mut event_pump = sdl_context.event_pump()?;
	let mut canvas = window.into_canvas();

	let mut x = 400;
	let mut y = 300;

	'running: loop {
		for event in event_pump.poll_iter() {
			match event {
				Event::Quit { .. } => break 'running,
				Event::KeyDown {
					keycode: Some(keycode),
					..
				} => match keycode {
					Keycode::W => y -= 10,
					Keycode::S => y += 10,
					Keycode::A => x -= 10,
					Keycode::D => x += 10,
					_ => {}
				},
				_ => {}
			}
		}

		canvas.set_draw_color(Color::RGB(0, 0, 0));
		canvas.clear();

		canvas.set_draw_color(Color::RGB(0xbb, 0x77, 0xff));
		canvas.draw_rect(Rect::from_center((x, y), 50, 50))?;

		canvas.present();

		thread::sleep(Duration::new(0, 1_000_000_000 / 60));
	}

	Ok(())
}
