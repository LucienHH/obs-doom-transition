#include <obs-module.h>
#include "version.h"

struct doom_info {
	obs_source_t *source;

	gs_effect_t *effect;
	gs_eparam_t *a_param;
	gs_eparam_t *b_param;
	gs_eparam_t *progress;
};

static const char *doom_get_name(void *type_data)
{
	UNUSED_PARAMETER(type_data);
	return obs_module_text("DoomTransition");
}

static void *doom_create(obs_data_t *settings, obs_source_t *source)
{
	struct doom_info *doom;
	char *file = obs_module_file("effects/doom_transition.effect");
	gs_effect_t *effect;

	obs_enter_graphics();
	effect = gs_effect_create_from_file(file, NULL);
	obs_leave_graphics();
	bfree(file);


	if (!effect) {
		blog(LOG_ERROR, "Could not find doom_transition.effect");
		return NULL;
	}

	doom = bmalloc(sizeof(*doom));
	doom->source = source;
	doom->effect = effect;
	doom->a_param = gs_effect_get_param_by_name(effect, "tex_a");
	doom->b_param = gs_effect_get_param_by_name(effect, "tex_b");
	doom->progress = gs_effect_get_param_by_name(effect, "progress");

	UNUSED_PARAMETER(settings);
	return doom;
}

static void doom_destroy(void *data)
{
	struct doom_info *doom = data;
	bfree(doom);
}

static void doom_callback(void *data, gs_texture_t *a, gs_texture_t *b, float t, uint32_t cx, uint32_t cy)
{
	struct doom_info *doom = data;

	const bool previous = gs_framebuffer_srgb_enabled();
	gs_enable_framebuffer_srgb(true);

	gs_effect_set_texture_srgb(doom->a_param, a);
	gs_effect_set_texture_srgb(doom->b_param, b);
	gs_effect_set_float(doom->progress, t);

	while (gs_effect_loop(doom->effect, "Doom"))
		gs_draw_sprite(NULL, 0, cx, cy);

	gs_enable_framebuffer_srgb(previous);


}

static void doom_video_render(void *data, gs_effect_t *effect)
{

	UNUSED_PARAMETER(effect);
	const bool previous = gs_set_linear_srgb(true);

	struct doom_info *doom = data;
	obs_transition_video_render(doom->source, doom_callback);

	gs_set_linear_srgb(previous);
}

static float mix_a(void *data, float t)
{
	UNUSED_PARAMETER(data);
	return 1.0f - t;
}

static float mix_b(void *data, float t)
{
	UNUSED_PARAMETER(data);
	return t;
}

static bool doom_audio_render(void *data, uint64_t *ts_out, struct obs_source_audio_mix *audio, uint32_t mixers,
			      size_t channels, size_t sample_rate)
{
	struct doom_info *doom = data;
	return obs_transition_audio_render(doom->source, ts_out, audio, mixers, channels, sample_rate, mix_a, mix_b);
}

static enum gs_color_space doom_video_get_color_space(void *data, size_t count, const enum gs_color_space *preferred_spaces)
{
	UNUSED_PARAMETER(count);
	UNUSED_PARAMETER(preferred_spaces);

	struct doom_info *const doom = data;
	return obs_transition_video_get_color_space(doom->source);
}

struct obs_source_info doom_transition = {
	.id = "doom_transition",
	.type = OBS_SOURCE_TYPE_TRANSITION,
	.get_name = doom_get_name,
	.create = doom_create,
	.destroy = doom_destroy,
	.video_render = doom_video_render,
	.audio_render = doom_audio_render,
	.video_get_color_space = doom_video_get_color_space,
};

OBS_DECLARE_MODULE()
OBS_MODULE_AUTHOR("LucienHH")
OBS_MODULE_USE_DEFAULT_LOCALE("obs-doom-transition", "en-US")

bool obs_module_load(void)
{
	blog(LOG_INFO, "[Doom Transition] loaded successfully (version %s)", PROJECT_VERSION);

  obs_register_source(&doom_transition);
  
	return true;
}

void obs_module_unload(void)
{
	blog(LOG_INFO, "plugin unloaded");
}
