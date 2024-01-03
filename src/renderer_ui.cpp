#include <renderer_ui.h>
#include <renderer_window.h>
#include <model.h>
#include <scene.h>
#include <scene_object.h>
#include <vector>

const char *glsl_version = "#version 150";
renderer_ui::renderer_ui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    selected = nullptr;
}

renderer_ui::~renderer_ui() { }

void renderer_ui::setup(GLFWwindow *window)
{
    ImGui::StyleColorsDark(); // Setup Dear ImGui style
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void renderer_ui::RenderAll(RendererWindow *window, Scene *scene)
{
    mainUI(window);
    resourceUI(window, scene);
    sceneUI(window, scene);
    detailUI(window, scene);
}

bool renderer_ui::isFocusOnUI() { return ImGui::GetIO().WantCaptureMouse; }

void renderer_ui::ImportModelPanel(RendererWindow *window)
{
    if (!showImportModelPanel)
    {
        return;
    }
    ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_FirstUseEver);
    {
        ImGui::Begin("Import Model");
        static char import_path[128] = "path..";
        static std::string info = "";
        ImGui::InputText("Model Path", import_path, IM_ARRAYSIZE(import_path));
        if (ImGui::Button("Cancel"))
        {
            showImportModelPanel = false;
            strcpy_s(import_path, "path..");
        }
        ImGui::SameLine();
        if (ImGui::Button("Confirm"))
        {
            std::string path_s = import_path;
            Model *new_model = new Model(path_s);
            if (new_model->meshes.size() == 0)
            {
                info = "Load failed";
            }
            else
            {
                showImportModelPanel = false;
                strcpy_s(import_path, "path..");
            }
        }
        ImGui::Text(info.c_str());
        ImGui::End();
    }
}

void renderer_ui::ImportShaderPanel(RendererWindow *window)
{
    if (!showImportShaderPanel)
    {
        return;
    }
    ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_FirstUseEver);
    {
        ImGui::Begin("Import Shader");

        static std::string info = "";

        static char vertex_path[128] = "vertex path..";
        ImGui::InputText("vertex Path", vertex_path, IM_ARRAYSIZE(vertex_path));

        static char frag_path[128] = "frag path..";
        ImGui::InputText("frag Path", frag_path, IM_ARRAYSIZE(frag_path));

        if (ImGui::Button("Cancel"))
        {
            showImportShaderPanel = false;
            strcpy_s(vertex_path, "vertex path..");
            strcpy_s(frag_path, "frag path..");
        }
        ImGui::SameLine();
        if (ImGui::Button("Confirm"))
        {
            std::string vert_path_s = vertex_path;
            std::string frag_path_s = frag_path;
            Shader *new_shader = new Shader(vert_path_s.c_str(), frag_path_s.c_str());
            new_shader->LoadShader();
            if (new_shader->ID == 0)
            {
                info = "Load failed";
            }
            else
            {
                showImportShaderPanel = false;
                strcpy_s(vertex_path, "vert path..");
                strcpy_s(frag_path, "frag path..");
            }
        }
        ImGui::Text(info.c_str());
        ImGui::End();
    }
}

void renderer_ui::ImportTexturePanel(RendererWindow *window)
{
    if (!showImportTexturePanel)
    {
        return;
    }
    ImGui::Begin("Import Texture");
    static char tex_path[128] = "path..";
    static std::string info = "";
    ImGui::InputText("Texture Path", tex_path, IM_ARRAYSIZE(tex_path));
    if (ImGui::Button("Cancel"))
    {
        showImportTexturePanel = false;
        strcpy_s(tex_path, "path..");
    }
    ImGui::SameLine();
    if (ImGui::Button("Confirm"))
    {
        std::string path_s = tex_path;
        Texture2D *new_tex = new Texture2D(path_s);
        if (!new_tex->is_valid)
        {
            info = "Load failed";
        }
        else
        {
            showImportTexturePanel = false;
            strcpy_s(tex_path, "path..");
        }
    }
    ImGui::Text(info.c_str());
    ImGui::End();
}


void renderer_ui::mainUI(RendererWindow *window)
{
    ImGuiWindowFlags window_flags = 0;
    bool *p_open = new bool(true);
    window_flags |= ImGuiWindowFlags_MenuBar;
    {
        ImGui::Begin("Main", p_open, window_flags);

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Files"))
            {
                if (ImGui::MenuItem("Import Model"))
                {
                    showImportModelPanel = true;
                }
                if (ImGui::MenuItem("Import Shader"))
                {
                    showImportShaderPanel = true;
                }
                if (ImGui::MenuItem("Import Texture"))
                {
                    showImportTexturePanel = true;
                }

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Options"))
            {
                ImGui::SliderFloat("Camera Speed", &window->render_camera->MovementSpeed, 0.0f, 10.0f);
                ImGui::ColorEdit3("clear color", (float *)window->clear_color); // Edit 3 floats representing a color
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGui::Text("Main Menu."); // Display some text (you can use a format strings too)
        ImGui::Text(" %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }
    ImportModelPanel(window);
    ImportShaderPanel(window);
    ImportTexturePanel(window);
}

void renderer_ui::sceneUI(RendererWindow *window, Scene *scene)
{
    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
    {
        ImGui::Begin("Scene");
        static int selected_obj = -1;
        std::vector<int> GC_Cache;
        if (selected_obj >= 0)
            selected = scene->scene_object_list[selected_obj];
        for (int n = 0; n < scene->scene_object_list.size(); n++)
        {
            std::string item_name = scene->scene_object_list[n]->name + "##" + std::to_string(scene->scene_object_list[n]->id);
            const char *item = item_name.c_str();
            if (ImGui::Selectable(item, selected_obj == n))
                selected_obj = n;
            if (ImGui::BeginPopupContextItem()) // <-- use last item id as popup id
            {
                selected_obj = n;
                ImGui::Text("This a popup for \"%s\"!", item);
                static char new_name[128] = "new name";
                ImGui::InputText("new name", new_name, IM_ARRAYSIZE(new_name));
                ImGui::SameLine();
                if (ImGui::Button("Rename"))
                {
                    scene->scene_object_list[n]->name = new_name;
                    strcpy_s(new_name, "new name");
                    ImGui::CloseCurrentPopup();
                }

                if (ImGui::Button("Remove"))
                {
                    selected = nullptr;
                    selected_obj = -1;
                    GC_Cache.push_back(n);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
            ImGui::SetItemTooltip("Right-click to open popup");
        }
        for (int i = 0; i < GC_Cache.size(); i++)
        {
            scene->RemoveSceneObjectAtIndex(GC_Cache[i]);
        }
        ImGui::End();
    }
}

void renderer_ui::detailUI(RendererWindow *window, Scene *scene)
{
    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
    {
        ImGui::Begin("Detail");
        if (selected != nullptr)
        {
            selected->RenderAttribute();
        }

        ImGui::End();
    }
}

void renderer_ui::resourceUI(RendererWindow *window, Scene *scene)
{
    // collect loaded resources' names
    std::vector<std::string> model_names;
    std::vector<std::string> tex_names;
    std::vector<std::string> shader_names;
    auto mmp = Model::LoadedModel;
    for (std::map<string, Model *>::iterator it = mmp.begin(); it != mmp.end(); it++)
    {
        model_names.push_back(it->first);
    }

    auto tmp = Texture2D::LoadedTextures;
    for (std::map<string, Texture2D *>::iterator it = tmp.begin(); it != tmp.end(); it++)
    {
        tex_names.push_back(it->first);
    }

    auto smp = Shader::LoadedShaders;
    for (std::map<string, Shader *>::iterator it = smp.begin(); it != smp.end(); it++)
    {
        shader_names.push_back(it->first);
    }

    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    {
        ImGui::Begin("Resource");
        float preview_width = window->width / 2;
        float preview_height = window->width / 8;
        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
        if (ImGui::BeginTabBar("Resource Bar", tab_bar_flags))
        {
            if (ImGui::BeginTabItem("Loaded Models"))
            {
                ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
                ImGui::BeginChild("ModelList", ImVec2(ImGui::GetContentRegionAvail().x - preview_width, preview_height - 20), ImGuiChildFlags_None, window_flags);
                static int selected_model = -1;
                for (int n = 0; n < model_names.size(); n++)
                {
                    if (ImGui::Selectable(model_names[n].c_str(), selected_model == n))
                        selected_model = n;
                    if (ImGui::BeginPopupContextItem()) // <-- use last item id as popup id
                    {
                        selected_model = n;
                        ImGui::Text("This a popup for \"%s\"!", model_names[n].c_str());
                        if (ImGui::Button("Create Instance"))
                        {
                            std::string name = model_names[n].substr(0, model_names[n].find_last_of('.'));
                            scene->InstanceFromModel(mmp[model_names[n]], name.c_str());
                            ImGui::CloseCurrentPopup();
                        }
                        if (ImGui::Button("Remove"))
                        {
                            selected_model = -1;
                            std::string name = model_names[n];
                            Model *tmp = Model::LoadedModel[name];
                            delete tmp;
                        }
                        if (ImGui::Button("Close"))
                            ImGui::CloseCurrentPopup();
                        ImGui::EndPopup();
                    }
                    ImGui::SetItemTooltip("Right-click to open popup");
                }
                ImGui::EndChild();

                ImGui::SameLine();

                if (selected_model >= 0)
                {
                    window_flags = ImGuiWindowFlags_None;
                    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
                    ImGui::BeginChild("ModelPreviewPanel", ImVec2(preview_width - 10, preview_height - 20), ImGuiChildFlags_Border, window_flags);
                    ImGui::BeginChild("ModelInfo", ImVec2(preview_width - 20, preview_height - 40), ImGuiChildFlags_Border);
                    Model *cur_model = Model::LoadedModel[model_names[selected_model]];
                    ImGui::Text(("name:" + cur_model->name).c_str());
                    ImGui::Text(("mesh count:" + std::to_string(cur_model->meshes.size())).c_str());
                    ImGui::Text(("ref count:" + std::to_string(cur_model->refSceneModels.references.size())).c_str());
                    ImGui::Text(("loaded textures:" + std::to_string(cur_model->textures_loaded.size())).c_str());
                    ImGui::Text(("directory:" + cur_model->directory).c_str());
                    ImGui::EndChild();
                    ImGui::EndChild();
                    ImGui::PopStyleVar();
                }
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Loaded Shaders"))
            {
                static int selected_shader = -1;
                for (int n = 0; n < shader_names.size(); n++)
                {
                    if (ImGui::Selectable(shader_names[n].c_str(), selected_shader == n))
                        selected_shader = n;
                    if (ImGui::BeginPopupContextItem()) // <-- use last item id as popup id
                    {
                        selected_shader = n;
                        ImGui::Text("This a popup for \"%s\"!", shader_names[n].c_str());
                        if (ImGui::Button("Refresh"))
                        {
                            Shader::LoadedShaders[shader_names[n]]->Refresh();
                            ImGui::CloseCurrentPopup();
                        }
                        if (ImGui::Button("Remove"))
                        {
                            Shader *tmp = Shader::LoadedShaders[shader_names[n]];
                            if (!tmp->is_editor)
                            {
                                selected_shader = -1;
                                delete tmp;
                            }
                            else
                            {
                                std::cout << "Editor Shader can not be removed!" << std::endl;
                            }
                        }
                        if (ImGui::Button("Close"))
                            ImGui::CloseCurrentPopup();
                        ImGui::EndPopup();
                    }
                    ImGui::SetItemTooltip("Right-click to open popup");
                }
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Loaded Textures"))
            {
                static int selected_tex = -1;
                {
                    ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
                    ImGui::BeginChild("TextureList", ImVec2(ImGui::GetContentRegionAvail().x - preview_width, preview_height - 20), ImGuiChildFlags_None, window_flags);
                    for (int n = 0; n < tex_names.size(); n++)
                    {
                        if (ImGui::Selectable(tex_names[n].c_str(), selected_tex == n))
                            selected_tex = n;
                        if (ImGui::BeginPopupContextItem()) // <-- use last item id as popup id
                        {
                            selected_tex = n;
                            ImGui::Text("This a popup for \"%s\"!", tex_names[n].c_str());
                            if (ImGui::Button("Remove"))
                            {
                                Texture2D *tmp = Texture2D::LoadedTextures[tex_names[n]];
                                if (!tmp->is_editor)
                                {
                                    selected_tex = -1;
                                    delete tmp;
                                }
                                else
                                {
                                    std::cout << "Editor Texture can not be removed!" << std::endl;
                                }
                            }
                            if (ImGui::Button("Close"))
                                ImGui::CloseCurrentPopup();
                            ImGui::EndPopup();
                        }
                        ImGui::SetItemTooltip("Right-click to open popup");
                    }
                    ImGui::EndChild();
                }

                ImGui::SameLine();
                {
                    if (selected_tex >= 0)
                    {
                        ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
                        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
                        ImGui::BeginChild("TexturePreviewPanel", ImVec2(preview_width - 10, preview_height - 20), ImGuiChildFlags_Border, window_flags);
                        Texture2D *tex = Texture2D::LoadedTextures[tex_names[selected_tex]];
                        ImGui::BeginChild("TextureInfo", ImVec2(preview_width - preview_height, preview_height - 40), ImGuiChildFlags_Border);
                        ImGui::Text(("name:" + tex->name).c_str());
                        ImGui::Text(("ref count:" + std::to_string(tex->textureRefs.references.size())).c_str());
                        ImGui::Text(("path:" + tex->path).c_str());
                        ImGui::Text(("type:" + tex->type).c_str());
                        ImGui::Text(("width:" + std::to_string(tex->width)).c_str());
                        ImGui::Text(("height:" + std::to_string(tex->height)).c_str());
                        ImGui::EndChild();

                        ImGui::SameLine();

                        ImGui::BeginChild("TexturePreview", ImVec2(preview_height - 40, preview_height - 40), ImGuiChildFlags_Border);
                        float width = preview_height - 60;
                        float height = preview_height - 60;
                        ImVec2 uv_min = ImVec2(0.0f, 0.0f);                        // Top-left
                        ImVec2 uv_max = ImVec2(1.0f, 1.0f);                        // Lower-right
                        ImVec4 tint_col = ImGui::GetStyleColorVec4(ImGuiCol_Text); // No tint
                        ImVec4 border_col = ImGui::GetStyleColorVec4(ImGuiCol_Border);
                        ImGui::Image((GLuint *)tex->id, ImVec2(width, height), uv_min, uv_max, tint_col, border_col);
                        ImGui::EndChild();
                        ImGui::EndChild();
                        ImGui::PopStyleVar();
                    }
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::End();
    }
}

void renderer_ui::shutdown()
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}