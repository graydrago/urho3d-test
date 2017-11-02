#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/StaticModel.h>

using namespace Urho3D;


class Main : public Application {
    URHO3D_OBJECT(Main, Application);

    protected:
        SharedPtr<Scene> m_scene;
        Node* m_box;
        float m_angle;

    public:
        Main(Context* ctx) : Application(ctx), m_angle(0) {};

        void Setup() {
            engineParameters_["FullScreen"]=false;
            engineParameters_["WindowWidth"]=1280;
            engineParameters_["WindowHeight"]=720;
            engineParameters_["WindowResizable"]=true;
        }

        void Start() {
            Camera* camera = nullptr;
            ResourceCache* cache = GetSubsystem<ResourceCache>();
            m_scene = SharedPtr<Scene>(new Scene(context_));
            m_scene->CreateComponent<Octree>();

            // Box
            {
                m_box = m_scene->CreateChild("Box");
                m_box->SetPosition({0, 0, 0});
                StaticModel* box = m_box->CreateComponent<StaticModel>();
                box->SetModel(cache->GetResource<Model>("some.mtl"));
                box->SetMaterial(cache->GetResource<Material>("Materials/DefaultGrey.xml"));
                box->SetCastShadows(true);
            }

            // Light
            {
                Node* node = m_scene->CreateChild("Light");
                node->SetDirection(Vector3::FORWARD);
                node->Yaw(50);
                node->Pitch(10);
                auto light = node->CreateComponent<Light>();
                light->SetLightType(LIGHT_DIRECTIONAL);
                light->SetBrightness(1.6);
                light->SetColor(Color::WHITE);
                light->SetCastShadows(true);
            }

            // camera
            {
                Node* node = m_scene->CreateChild("Camera");
                node->SetPosition({10, 10, 10});
                node->LookAt({0, 0, 0});
                camera = node->CreateComponent<Camera>();
                camera->SetFarClip(10000);
            }

            auto renderer = GetSubsystem<Renderer>();
            SharedPtr<Viewport> viewport(new Viewport(context_, m_scene, camera));
            renderer->SetViewport(0, viewport);

            SubscribeToEvents();
        }


    void SubscribeToEvents()
    {
        // Subscribe HandleUpdate() function for processing update events
        SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Main, HandleUpdate));
    }


    void HandleUpdate(StringHash eventType, VariantMap& eventData) {
        using namespace Update;
        //float timeStep = eventData[P_TIMESTEP].GetFloat();
        m_box->SetRotation({0, m_angle, 0});
        m_angle += 0.4;
    }
};


URHO3D_DEFINE_APPLICATION_MAIN(Main)

