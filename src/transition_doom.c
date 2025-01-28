#include <obs-module.h>
#include "version.h"

struct doom_info {
	obs_source_t *source;

	gs_effect_t *effect;
	gs_eparam_t *a_param;
	gs_eparam_t *b_param;
	gs_eparam_t *progress;
	gs_eparam_t *bars_param;
	gs_eparam_t *frequency_param;
	gs_eparam_t *drip_scale_param;
	gs_eparam_t *noise_param;
	gs_eparam_t *amplitude_param;

	gs_texture_t *first_frame;
	bool first_frame_set;

	int bars;
	float frequency;
	float drip_scale;
	float noise;
	float amplitude;
	bool freeze_frame;

};

#define S_BARS "bars"
#define S_FREQUENCY "frequency"
#define S_DRIP_SCALE "drip_scale"
#define S_NOISE "noise"
#define S_AMPLITUDE "amplitude"
#define S_FREEZE_FRAME "freeze_frame"
#define S_RANDOMISE_BARS "randomise_bars"

#define S_BARS_TEXT obs_module_text("Bars")
#define S_FREQUENCY_TEXT obs_module_text("Frequency")
#define S_DRIP_SCALE_TEXT obs_module_text("DripScale")
#define S_NOISE_TEXT obs_module_text("Noise")
#define S_AMPLITUDE_TEXT obs_module_text("Amplitude")
#define S_FREEZE_FRAME_TEXT obs_module_text("FreezeFrame")
#define S_RANDOMISE_BARS_TEXT obs_module_text("RandomiseBars")

static const char *doom_get_name(void *type_data)
{
	UNUSED_PARAMETER(type_data);
	return obs_module_text("DoomTransition");
}

static void doom_update(void *data, obs_data_t *settings)
{
	struct doom_info *doom = data;

	doom->bars = (int)obs_data_get_int(settings, S_BARS);
	doom->frequency = (float)obs_data_get_double(settings, S_FREQUENCY);
	doom->drip_scale = (float)obs_data_get_double(settings, S_DRIP_SCALE);
	doom->noise = (float)obs_data_get_double(settings, S_NOISE);
	doom->amplitude = (float)obs_data_get_double(settings, S_AMPLITUDE);

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
	doom->bars_param = gs_effect_get_param_by_name(effect, "bars");
	doom->frequency_param = gs_effect_get_param_by_name(effect, "frequency");
	doom->drip_scale_param = gs_effect_get_param_by_name(effect, "drip_scale");
	doom->noise_param = gs_effect_get_param_by_name(effect, "noise");
	doom->amplitude_param = gs_effect_get_param_by_name(effect, "amplitude");

	doom->first_frame = NULL;
	doom->first_frame_set = false;

	obs_source_update(source, settings);

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

	if (!doom->first_frame_set) {
		doom->first_frame = gs_texture_create(gs_texture_get_width(a), gs_texture_get_height(a), GS_RGBA, 1, NULL, GS_DYNAMIC);
		gs_copy_texture(doom->first_frame, a);
		doom->first_frame_set = true;
	}

	gs_effect_set_texture_srgb(doom->a_param, doom->freeze_frame ? doom->first_frame : a);
	gs_effect_set_texture_srgb(doom->b_param, b);
	gs_effect_set_float(doom->progress, t);
	gs_effect_set_int(doom->bars_param, doom->bars);
	gs_effect_set_float(doom->frequency_param, doom->frequency);
	gs_effect_set_float(doom->drip_scale_param, doom->drip_scale);
	gs_effect_set_float(doom->noise_param, doom->noise);
	gs_effect_set_float(doom->amplitude_param, doom->amplitude);

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

static obs_properties_t *doom_properties(void *data)
{
	obs_properties_t *props = obs_properties_create();

	obs_properties_add_bool(props, S_FREEZE_FRAME, S_FREEZE_FRAME_TEXT);
	obs_properties_add_int_slider(props, S_BARS, S_BARS_TEXT, 0, 100, 1);
	obs_properties_add_float_slider(props, S_FREQUENCY, S_FREQUENCY_TEXT, 0.0, 1.0, 0.01);
	obs_properties_add_float_slider(props, S_DRIP_SCALE, S_DRIP_SCALE_TEXT, 0.0, 1.0, 0.01);
	obs_properties_add_float_slider(props, S_NOISE, S_NOISE_TEXT, 0.0, 1.0, 0.01);
	obs_properties_add_float_slider(props, S_AMPLITUDE, S_AMPLITUDE_TEXT, 0.0, 10.0, 0.01);

	UNUSED_PARAMETER(data);
	return props;
}

static void doom_defaults(obs_data_t *settings)
{
 	obs_data_set_default_int(settings, S_BARS, (int)30);
 	obs_data_set_default_double(settings, S_FREQUENCY, (double)0.5);
 	obs_data_set_default_double(settings, S_DRIP_SCALE, (double)0.5);
 	obs_data_set_default_double(settings, S_NOISE, (double)0.1);
 	obs_data_set_default_double(settings, S_AMPLITUDE, (double)2.0);
	obs_data_set_default_bool(settings, S_FREEZE_FRAME, true);
}

static void doom_transition_start(void *data)
{

	struct doom_info *doom = data;

	if (doom->first_frame) {
		gs_texture_destroy(doom->first_frame);
		doom->first_frame = NULL;
		doom->first_frame_set = false;
	}

}

static void doom_transition_stop(void *data)
{

	struct doom_info *doom = data;

	if (doom->first_frame) {
		gs_texture_destroy(doom->first_frame);
		doom->first_frame = NULL;
		doom->first_frame_set = false;
	}

}

struct obs_source_info doom_transition = {
	.id = "doom_transition",
	.type = OBS_SOURCE_TYPE_TRANSITION,
	.get_name = doom_get_name,
	.create = doom_create,
	.destroy = doom_destroy,
	.update = doom_update,
	.video_render = doom_video_render,
	.audio_render = doom_audio_render,
	.get_defaults = doom_defaults,
	.get_properties = doom_properties,
	.transition_start = doom_transition_start,
	.transition_stop = doom_transition_stop,
	.video_get_color_space = doom_video_get_color_space,
};

struct doom_faithful_info {
	obs_source_t *source;

	gs_effect_t *effect;
	gs_eparam_t *a_param;
	gs_eparam_t *b_param;
	gs_eparam_t *progress;
	gs_eparam_t *seed;

	gs_texture_t *first_frame;
	bool first_frame_set;

	bool freeze_frame;
	bool randomise_bars;
	float seed_value;
};

static const char *doom_faithful_get_name(void *type_data)
{
	UNUSED_PARAMETER(type_data);
	return obs_module_text("DoomFaithfulTransition");
}

static void *doom_faithful_create(obs_data_t *settings, obs_source_t *source)
{
	struct doom_faithful_info *doom_faithful;
	char *file = obs_module_file("effects/doom_faithful_transition.effect");
	gs_effect_t *effect;

	obs_enter_graphics();
	effect = gs_effect_create_from_file(file, NULL);
	obs_leave_graphics();
	bfree(file);

	if (!effect) {
		blog(LOG_ERROR, "Could not find doom_faithful_transition.effect");
		return NULL;
	}

	doom_faithful = bmalloc(sizeof(*doom_faithful));
	doom_faithful->source = source;
	doom_faithful->effect = effect;
	doom_faithful->a_param = gs_effect_get_param_by_name(effect, "tex_a");
	doom_faithful->b_param = gs_effect_get_param_by_name(effect, "tex_b");
	doom_faithful->progress = gs_effect_get_param_by_name(effect, "progress");
	doom_faithful->seed = gs_effect_get_param_by_name(effect, "seed");

	doom_faithful->first_frame = NULL;
	doom_faithful->first_frame_set = false;

	UNUSED_PARAMETER(settings);

	return doom_faithful;
}

static void doom_faithful_destroy(void *data)
{
	struct doom_faithful_info *doom_faithful = data;
	bfree(doom_faithful);
}

static void doom_faithful_update(void *data, obs_data_t *settings)
{
	struct doom_faithful_info *doom_faithful = data;

	doom_faithful->freeze_frame = obs_data_get_bool(settings, S_FREEZE_FRAME);
	doom_faithful->randomise_bars = obs_data_get_bool(settings, S_RANDOMISE_BARS);
}

static void doom_faithful_callback(void *data, gs_texture_t *a, gs_texture_t *b, float t, uint32_t cx, uint32_t cy)
{
	struct doom_faithful_info *doom_faithful = data;

	const bool previous = gs_framebuffer_srgb_enabled();
	gs_enable_framebuffer_srgb(true);

	if (!doom_faithful->first_frame_set) {
		doom_faithful->first_frame = gs_texture_create(gs_texture_get_width(a), gs_texture_get_height(a), GS_RGBA, 1, NULL, GS_DYNAMIC);
		gs_copy_texture(doom_faithful->first_frame, a);
		doom_faithful->first_frame_set = true;
	}

	if (doom_faithful->seed_value == 0.0f) {
		doom_faithful->seed_value = (float)rand() / (float)RAND_MAX * (float)gs_texture_get_width(a);
	}

	gs_effect_set_texture_srgb(doom_faithful->a_param, doom_faithful->freeze_frame ? doom_faithful->first_frame : a);
	gs_effect_set_texture_srgb(doom_faithful->b_param, b);
	gs_effect_set_float(doom_faithful->progress, t);

	gs_effect_set_float(doom_faithful->seed, doom_faithful->randomise_bars ? doom_faithful->seed_value : 0.0f);

	while (gs_effect_loop(doom_faithful->effect, "DoomFaithful"))
		gs_draw_sprite(NULL, 0, cx, cy);

	gs_enable_framebuffer_srgb(previous);

}

static void doom_faithful_video_render(void *data, gs_effect_t *effect)
{
	UNUSED_PARAMETER(effect);
	const bool previous = gs_set_linear_srgb(true);

	struct doom_faithful_info *doom_faithful = data;
	obs_transition_video_render(doom_faithful->source, doom_faithful_callback);

	gs_set_linear_srgb(previous);
}

static bool doom_faithful_audio_render(void *data, uint64_t *ts_out, struct obs_source_audio_mix *audio, uint32_t mixers, size_t channels, size_t sample_rate)
{
	struct doom_faithful_info *doom_faithful = data;
	return obs_transition_audio_render(doom_faithful->source, ts_out, audio, mixers, channels, sample_rate, mix_a, mix_b);
}

static enum gs_color_space doom_faithful_video_get_color_space(void *data, size_t count, const enum gs_color_space *preferred_spaces)
{
	UNUSED_PARAMETER(count);
	UNUSED_PARAMETER(preferred_spaces);
	struct doom_faithful_info *const doom_faithful = data;
	return obs_transition_video_get_color_space(doom_faithful->source);
}

static void doom_faithful_transition_start(void *data)
{

	struct doom_faithful_info *doom_faithful = data;

	if (doom_faithful->first_frame) {
		gs_texture_destroy(doom_faithful->first_frame);
		doom_faithful->first_frame = NULL;
		doom_faithful->first_frame_set = false;
	}

	doom_faithful->seed_value = 0.0f;

}

static void doom_faithful_transition_stop(void *data)
{

	struct doom_faithful_info *doom_faithful = data;

	if (doom_faithful->first_frame) {
		gs_texture_destroy(doom_faithful->first_frame);
		doom_faithful->first_frame = NULL;
		doom_faithful->first_frame_set = false;
	}

	doom_faithful->seed_value = 0.0f;

}

static obs_properties_t *doom_faithful_properties(void *data)
{
	obs_properties_t *props = obs_properties_create();

	obs_properties_add_bool(props, S_FREEZE_FRAME, S_FREEZE_FRAME_TEXT);
	obs_properties_add_bool(props, S_RANDOMISE_BARS, S_RANDOMISE_BARS_TEXT);

	UNUSED_PARAMETER(data);
	return props;
}

static void doom_faithful_defaults(obs_data_t *settings)
{
	obs_data_set_default_bool(settings, S_FREEZE_FRAME, true);
	obs_data_set_default_bool(settings, S_RANDOMISE_BARS, true);
}

struct obs_source_info doom_faithful_transition = {
	.id = "doom_faithful_transition",
	.type = OBS_SOURCE_TYPE_TRANSITION,
	.get_name = doom_faithful_get_name,
	.create = doom_faithful_create,
	.destroy = doom_faithful_destroy,
	.update = doom_faithful_update,
	.video_render = doom_faithful_video_render,
	.audio_render = doom_faithful_audio_render,
	.transition_start = doom_faithful_transition_start,
	.transition_stop = doom_faithful_transition_stop,
	.get_defaults = doom_faithful_defaults,
	.get_properties = doom_faithful_properties,
	.video_get_color_space = doom_faithful_video_get_color_space,
};

OBS_DECLARE_MODULE()
OBS_MODULE_AUTHOR("LucienHH")
OBS_MODULE_USE_DEFAULT_LOCALE("obs-doom-transition", "en-US")

bool obs_module_load(void)
{
	blog(LOG_INFO, "[Doom Transition] loaded successfully (version %s)", PROJECT_VERSION);

  obs_register_source(&doom_transition);
	obs_register_source(&doom_faithful_transition);
  
	return true;
}

void obs_module_unload(void)
{
	blog(LOG_INFO, "plugin unloaded");
}
