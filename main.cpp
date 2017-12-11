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
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Math/MathDefs.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/StaticModel.h>

using namespace Urho3D;


class Main : public Application {
    URHO3D_OBJECT(Main, Application);

    protected:
        SharedPtr<Scene> m_scene;
        Node* m_box;
        Node* m_floor;
        Node* m_camera_node;
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
            m_scene->CreateComponent<PhysicsWorld>();
            m_scene->CreateComponent<DebugRenderer>();

            // Box
            {
                m_box = m_scene->CreateChild("Box");
                m_box->SetPosition({0, 0, 0});
                StaticModel* box = m_box->CreateComponent<StaticModel>();
                box->SetModel(cache->GetResource<Model>("box.mtl"));
                box->SetMaterial(cache->GetResource<Material>("Materials/Material.xml"));
                box->SetCastShadows(true);
                m_box->CreateComponent<RigidBody>();
                auto* shape = m_box->CreateComponent<CollisionShape>();
                shape->SetBox({1, 1, 1});
            }

            // Floor
            {
                m_floor = m_scene->CreateChild("Floor");
                m_floor->SetPosition({0, -1, 0});
                m_floor->SetScale(200);
                m_floor->SetRotation({90, 0, 0});
                StaticModel* model = m_floor->CreateComponent<StaticModel>();
                model->SetModel(cache->GetResource<Model>("plane.mtl"));
                model->SetMaterial(cache->GetResource<Material>("Materials/Material.xml"));
                model->SetCastShadows(true);
                /*auto* body = */m_floor->CreateComponent<RigidBody>();
                auto* shape = m_floor->CreateComponent<CollisionShape>();
                //shape->SetStaticPlane(m_floor->GetPosition(), m_floor->GetRotation());
                shape->SetTriangleMesh(cache->GetResource<Model>("plane.mtl"));//,
                                       //0,
                                       //m_floor->GetScale(),
                                       //m_floor->GetPosition(),
                                       //m_floor->GetRotation());
            }

            // Light
            {
                Node* node = m_scene->CreateChild("Light");
                node->SetDirection(Vector3::DOWN);
                node->Yaw(-30);
                node->Pitch(-30);
                auto light = node->CreateComponent<Light>();
                light->SetLightType(LIGHT_DIRECTIONAL);
                light->SetBrightness(1.6);
                light->SetColor(Color::WHITE);
                light->SetCastShadows(true);
            }

            // camera
            {
                Node* t_node = m_scene->CreateChild("Camera_t");
                Node* r_node = t_node->CreateChild("Camera_r");
                t_node->SetPosition({0, 10, -30});
                r_node->LookAt({0, 10, 0});
                camera = r_node->CreateComponent<Camera>();
                camera->SetFarClip(10000);
                RigidBody* body = t_node->CreateComponent<RigidBody>();
                body->SetMass(50);
                body->SetFriction(0.1);
                body->SetAngularFactor({0, 0, 0});
                body->SetLinearDamping(0.2);
                CollisionShape* shape = t_node->CreateComponent<CollisionShape>();
                shape->SetCapsule(2, 2);
                m_camera_node = t_node;
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


    void HandleUpdate(StringHash /*eventType*/, VariantMap& /*eventData*/) {
        using namespace Update;

        auto r_camera = m_camera_node->GetChildren()[0];
        auto* input = GetSubsystem<Input>();
        auto smoothDeltaTime = GetSubsystem<Engine>()->GetNextTimeStep();
        auto mouse_offset = input->GetMouseMove();
        auto rot = r_camera->GetRotation();

        // rotation
        rot.FromEulerAngles(
          Clamp(rot.PitchAngle() + mouse_offset.y_ * smoothDeltaTime * 10.f, -80.f, 80.f),
          rot.YawAngle() + mouse_offset.x_ * smoothDeltaTime * 10.f,
          rot.RollAngle()
        );
        r_camera->SetRotation(rot);

        // position
        auto cam_pos = m_camera_node->GetPosition();
        RigidBody* rb = m_camera_node->GetComponent<RigidBody>();
        Vector3 direction{0,0,0};
        float move_speed = (input->GetKeyDown(KEY_SHIFT)) ? 3000 : 1500;
        if (input->GetKeyDown(KEY_W)) direction.z_ =  1;
        if (input->GetKeyDown(KEY_S)) direction.z_ =  -1;
        if (input->GetKeyDown(KEY_A)) direction.x_ =  -1;
        if (input->GetKeyDown(KEY_D)) direction.x_ =  1;
        PODVector<RigidBody *> collisions;
        rb->GetCollidingBodies(collisions);
        if (collisions.Size() > 0) {
          if (input->GetKeyDown(KEY_SPACE)) rb->ApplyImpulse({0, 60, 0});
        }
        direction.Normalize();
        auto cm = rot * direction * smoothDeltaTime * move_speed;
        cm.y_ = rb->GetLinearVelocity().y_;
        rb->SetLinearVelocity(cm);
    }
};


URHO3D_DEFINE_APPLICATION_MAIN(Main)

