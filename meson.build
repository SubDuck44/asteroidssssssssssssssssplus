project(
    'asteroidssssssssssssssssplus',
    'c',
    default_options: {
        'warning_level': '3',
        'c_std': 'c23',
    },
)

res = custom_target(
    'res',
    output: 'res.c',
    command: ['src/gen_res.sh', '@OUTPUT@'],
)

executable(
    'asteroids16plus.x86_64',

    'src/main.c',
	'src/engine.c',
	'src/gameobjects.c',
	res,

    dependencies: [
        dependency('sdl3'),
        dependency('sdl3-ttf'),
        dependency('sdl3-gfx'),
    ],

	c_args: [
        '-Wno-empty-translation-unit',
        '-Wno-pragma-once-outside-header',
    ],

    link_args: [
        '-lm',
    ],

    install: true,
)
