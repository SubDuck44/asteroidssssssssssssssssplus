project(
	'asteroidssssssssssssssssplus',
	'c',
	default_options: {
		'buildtype': 'debugoptimized',
		'warning_level': '3',
		'c_std': 'c23',
		'b_ndebug': 'if-release',
	},
)

res = configure_file(
	output: 'res.c',
	command: ['src/gen_res.sh'],
	capture: true,
)

objects = configure_file(
	output: 'objects.c',
	command: ['src/gen_objects.sh'],
	capture: true,
)

executable(
	'asteroids16plus.x86_64',
	sources: [
		res,
		objects,
		run_command('find', 'src', '-name', '*.c', check: true).stdout().split(),
	],
	include_directories: [
		include_directories('src'),
	],
	dependencies: [
		dependency('la'),
		dependency('sdl3'),
		dependency('sdl3-gfx'),
		dependency('sdl3-ttf'),
	],
	c_args: [
		'-D_DEFAULT_SOURCE',
		'-Wno-empty-translation-unit',
		'-Wno-pragma-once-outside-header',
	],
	install: true,
)
