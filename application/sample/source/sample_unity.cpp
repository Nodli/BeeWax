struct Sample_Scene{
    Sample_Scene(){
    }
    ~Sample_Scene(){
    }
    void update(){
    }
    void render(){
    }
};


void user_config(){}

void* user_create(){
    g_engine.scene_manager.push_scene<Sample_Scene>("Sample_Scene");
    return nullptr;
}

void user_destroy(void*){}
