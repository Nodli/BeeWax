
// REF(hugo): plugins
// http://www.martin-finke.de/blog/tags/making_audio_plugins.html

// REF(hugo): theremin
// https://www.soundonsound.com/techniques/reproducing-theremin-sounds-using-synthesizer
// https://www.soundonsound.com/techniques/synthesizing-bowed-strings-violin-family
// http://www.thereminworld.com/Forums/T/32167/theremin-like-sound-synthesis
// http://www.thereminworld.com/Forums/T/27423/etherwave-waveform-knob-question
// https://www.kvraudio.com/forum/viewtopic.php?t=462380
// http://www.electrotheremin.com/Topic-sound.html
// https://www.youtube.com/watch?v=-fK7d76tqqY
// https://www.youtube.com/watch?v=k4yDksx3l2U

// http://web.physics.ucsb.edu/~lecturedemonstrations/Composer/Pages/60.17.html
// "the beat frequency can be anywhere from around 65 Hz to about 3 kHz"

// https://www.youtube.com/watch?v=f_BDjCEoWzU

// http://theremin.tf/wp-content/uploads/2015/03/wavegen.pdf
// http://www.thereminworld.com/Forums/T/30784/finding-claras-voice-she-is-vocal
// https://www.desmos.com/calculator/3dswge7n3t

// http://www.thereminworld.com/album/clara-rockmore-theremin-replica/15672?photo=15673
// range in [-2.0 ;+2.5]
// * x1 = x - 1
// * x2 = x - 2
// * x3 = x - 2.5
// * x4 = x - 3
// * c1 = 0.5 * (x1 * x1) - 2   {0 < x < 2}
// * c2 = -1.5 + x2 * 7         {2 < x < 2.5}
// * c3 = 2 + x3                {2.5 < x < 3}
// * c4 = 2.5 - x4 * 8          {3 < x < 3.5}

// https://www.desmos.com/calculator/jnuqmcl3gp

// https://pages.mtu.edu/~suits/NoteFreqCalcs.html

float generate_sine_wave(float* buffer, s32 buffer_size, float phase,
        float wave_amplitude, u32 wave_frequency, u32 samples_per_second){

    constexpr float TWO_PI = 2.f * PI;
    float phase_per_sample = TWO_PI * (float)wave_frequency / (float)samples_per_second;

    for(u32 isamp = 0u; isamp != (u32)buffer_size; ++isamp){
        float sample = bw::sin(phase);

        sample *= (- SHRT_MIN) * wave_amplitude;
        buffer[isamp] += sample;

        phase += phase_per_sample;
        if(phase > TWO_PI){
            phase -= TWO_PI * (u32)(phase / TWO_PI);
        }
    }

    return phase;
}

void generate_theremin_wave(float* buffer, s32 buffer_size, float& wave_phase, float& beat_phase,
        float wave_amplitude, u32 wave_frequency, u32 beat_frequency, u32 samples_per_second){

    constexpr float oscillator_period = 2.f * PI;

    float beat_phase_per_sample = oscillator_period * (float)beat_frequency / (float)samples_per_second;

    for(u32 isamp = 0u; isamp != (u32)buffer_size; ++isamp){
        float sample_frequency = wave_frequency + (0.5f * bw::sin(0.25f * beat_phase) - 0.5f) * (1.f/64.f) * wave_frequency;
        float wave_phase_per_sample = oscillator_period * (float)sample_frequency / (float)samples_per_second;

        //float sample = tanh(bw::asin(bw::sin(wave_phase)) * 5.f)
        //    + tanh(bw::asin(bw::sin(2.f * wave_phase)) * (1.f / 4.f))
        //    + bw::sin(3.f * wave_phase) * (1.f / 32.f)
        //    + tanh(bw::asin(bw::sin(4.f * wave_phase)) * (1.f / 16.f));
        //    + bw::sin(5.f * wave_phase) * (1.f / 16.f);

        //sample *= 0.75f + bw::sin(beat_phase) * 0.25f;

        float sample = tanh(
                bw::asin(bw::sin(wave_phase)) + 1.f
                - (1.f / 4.f) * bw::asin(bw::sin(3.f * wave_phase))
        );
        sample *= 0.5 + 0.25f * bw::sin(beat_phase) + 0.25f;

        sample *= (- SHRT_MIN) * wave_amplitude;

        buffer[isamp] += sample;

        wave_phase += wave_phase_per_sample;
        if(wave_phase > oscillator_period){
            wave_phase -= oscillator_period * (u32)(wave_phase / oscillator_period);
        }

        beat_phase += beat_phase_per_sample;
        if(beat_phase > oscillator_period){
            beat_phase -= oscillator_period * (u32)(beat_phase / oscillator_period);
        }
    }
}

struct Theremin{

    struct State{
        float wave_phase;
        float beat_phase;
    };

    struct Parameters{
        u32 generate_sound;
        u32 samples_per_second;
        u32 wave_frequency;
        u32 beat_frequency;
        float wave_amplitude;
    };

    // --

    void create(){
        memset(param_array, 0x00, sizeof(param_array));
        atomic_set(&reader_index, 2u);
        atomic_set(&writer_index, 0u);

        state.wave_phase = 0.f;
        state.beat_phase = 0.f;
    }
    void destroy(){
    }

    // --

    static u32 previous_index(u32 index){
        return (index + 2u) % 3u;
    }

    static u32 next_index(u32 index){
        return (index + 1u) % 3u;
    }

    Parameters& get_parameters(){
        return param_array[writer_index];
    }

    // NOTE(hugo): don't call more often than the audio callback otherwise data won't be commited
    void commit_parameters(){
        u32 rindex = atomic_get(&reader_index);
        if(rindex == previous_index(writer_index)){
            u32 new_index = next_index(writer_index);
            param_array[new_index] = param_array[writer_index];
            writer_index = new_index;
        }
    }

    void process(float* buffer, s32 buffer_size){
        u32 windex = atomic_get(&writer_index);
        if(windex != next_index(reader_index)) reader_index = next_index(reader_index);

        Parameters param = param_array[reader_index];

        if(param.generate_sound){
            //state.phase = generate_sine_wave(buffer, buffer_size, state.phase, param.wave_amplitude, param.wave_frequency, param.samples_per_second);
            generate_theremin_wave(buffer, buffer_size, state.wave_phase, state.beat_phase, param.wave_amplitude, param.wave_frequency, param.beat_frequency, param.samples_per_second);
        }
    }

    // ----

    u32 reader_index;
    u32 writer_index;
    Parameters param_array[3u];
    State state;
};

struct Theremin_Scene{
    Theremin_Scene(){
        theremin.create();

        auto theremin_process = [](void* data, float* buffer, s32 buffer_size){
            Theremin* tptr = (Theremin*)data;
            (*tptr).process(buffer, buffer_size);
        };
        synth = get_engine().audio.start(&theremin, theremin_process);

        Theremin::Parameters& param = theremin.get_parameters();
        param.generate_sound = true;
        param.samples_per_second = 44100u;
        param.wave_frequency = 440u;
        param.beat_frequency = param.wave_frequency;
        param.wave_amplitude = 0.5f;

        theremin.commit_parameters();

        get_engine().action_manager.register_action(0u, MOUSE_POSITION);
    }
    ~Theremin_Scene(){
        get_engine().audio.stop(synth);
        theremin.destroy();
    }
    void update(){
        Action_Data action = get_engine().action_manager.get_action(0u);
        float action_x = (float)action.cursor.position_x / (float)get_engine().window.width;
        float action_y = (float)action.cursor.position_y / (float)get_engine().window.height;

        Theremin::Parameters& param = theremin.get_parameters();
        //param.wave_frequency = 65u + action_x * (3000u - 65u);
        param.wave_frequency = 179u;
        param.beat_frequency = 7u;
        param.wave_amplitude = (action_y + (1.f - action_x)) * 0.25f;
        theremin.commit_parameters();
    }
    void render(){}

    // ----

    Synth_Channel synth;
    Theremin theremin;
};
