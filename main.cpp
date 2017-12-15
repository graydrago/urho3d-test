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
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Math/MathDefs.h>
#include <Urho3D/Math/Ray.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Sprite.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <unordered_map>
#include <vector>
#include <cmath>

using namespace Urho3D;


struct InputState {
    bool just_pressed = false;
    bool still_pressed = false;
    bool just_released = false;
    bool still_released = true;
};


class Main : public Application {
    URHO3D_OBJECT(Main, Application);

    protected:
        SharedPtr<Scene> m_scene;
        Node* m_box;
        Node* m_floor;
        Node* m_camera_node;
        Text* m_text;
        Camera* m_camera;
        std::unordered_map<int, InputState> m_input_state;
        std::vector<Node*> m_boxes;

        enum class player_state { jump, fall, stay };
        float m_angle;


    public:
        Main(Context* ctx) : Application(ctx), m_angle(0) {};

        void Setup() {
            engineParameters_["FullScreen"]=false;
            engineParameters_["WindowWidth"]=1280;
            engineParameters_["WindowHeight"]=720;
            engineParameters_["WindowResizable"]=true;

            InputState some;
            m_input_state.insert({KEY_W, some});
            m_input_state.insert({KEY_A, some});
            m_input_state.insert({KEY_S, some});
            m_input_state.insert({KEY_D, some});
            m_input_state.insert({KEY_SPACE, some});
            m_input_state.insert({MOUSEB_LEFT, some});
        }

        void Start() {
            Camera* camera = nullptr;
            ResourceCache* cache = GetSubsystem<ResourceCache>();
            m_scene = SharedPtr<Scene>(new Scene(context_));
            m_scene->CreateComponent<Octree>();
            m_scene->CreateComponent<PhysicsWorld>();
            m_scene->CreateComponent<DebugRenderer>();

            // Box
            auto yellow = cache->GetResource<Material>("Materials/Material.xml")->Clone();
            yellow->SetShaderParameter("MatDiffColor", Color(1.f, 0.86f, 0.f));
            for (int i = 0; i < 10; i++) {
                m_box = m_scene->CreateChild("Box");
                m_box->SetPosition({i * 3.f, 5, 0});
                StaticModel* box = m_box->CreateComponent<StaticModel>();
                box->SetModel(cache->GetResource<Model>("box.mtl"));
                auto m = cache->GetResource<Material>("Materials/Material.xml")->Clone();
                m->SetShaderParameter("MatDiffColor", Color(0, i/10.f, 0));
                //box->SetMaterial(cache->GetResource<Material>("Materials/Material.xml"));
                box->SetMaterial(m);
                //box->SetCastShadows(true);
                auto body = m_box->CreateComponent<RigidBody>();
                body->SetMass(1000);
                body->SetFriction(1);
                auto* shape = m_box->CreateComponent<CollisionShape>();
                shape->SetBox({2, 2, 2});
                m_boxes.push_back(m_box);

                auto es_node = m_box->CreateChild("Exclamation Mark");
                Quaternion q;
                q.FromAngleAxis(90, {1, 0, 0});
                es_node->SetRotation(q);
                es_node->SetPosition({0, 1.2, 0});
                auto es = es_node->CreateComponent<StaticModel>();
                es->SetModel(cache->GetResource<Model>("exclamation_mark.mtl"));
                es->SetMaterial(yellow);
            }

            {
                UI* ui = GetSubsystem<UI>();
                m_text = ui->GetRoot()->CreateChild<Text>();
                m_text->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);
                m_text->SetHorizontalAlignment(HA_LEFT);
                m_text->SetVerticalAlignment(VA_TOP);
                m_text->SetPosition(100, 100);

                BorderImage* yourImage_ = ui->GetRoot()->CreateChild<BorderImage>();
                auto* t = cache->GetResource<Texture2D>("Textures/aim.png");
                yourImage_->SetTexture(t);
                yourImage_->SetSize(10,10);
                yourImage_->SetAlignment(HA_CENTER,VA_CENTER);
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
                shape->SetTriangleMesh(cache->GetResource<Model>("plane.mtl"));//,
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
                shape->SetCapsule(1.5, 1);
                shape->SetMargin(5);
                m_camera_node = t_node;
                m_camera = camera;
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

        auto* input = GetSubsystem<Input>();

        for (auto& is_pair : m_input_state) {
            auto& is = is_pair.second;
            if (input->GetKeyDown(is_pair.first)) {
                if (is.just_released || is.still_released) {
                    is.just_pressed = true;
                    is.still_pressed = false;
                    is.just_released = false;
                    is.still_released = false;
                } else if (is.just_pressed) {
                    is.just_pressed = false;
                    is.still_pressed = true;
                    is.just_released = false;
                    is.still_released = false;
                }
            } else {
                if (is.just_pressed || is.still_pressed) {
                    is.just_pressed = false;
                    is.still_pressed = false;
                    is.just_released = true;
                    is.still_released = false;
                } else {
                    is.just_pressed = false;
                    is.still_pressed = false;
                    is.just_released = false;
                    is.still_released = true;
                }
            };
        }

        auto r_camera = m_camera_node->GetChildren()[0];
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
        if (m_input_state[KEY_SPACE].just_pressed && collisions.Size() > 0) {
          if (input->GetKeyDown(KEY_SPACE)) rb->ApplyImpulse({0, 400, 0});
          m_text->SetText("Jump");
        }

        //if (m_input_state[MOUSEB_LEFT].just_pressed && collisions.Size() > 0) {
            //m_text->SetText("Shot");
        //} else {
            //m_text->SetText("!!!");
        //}
        //
        if (input->GetMouseButtonPress(MOUSEB_LEFT)) {
            auto ray = m_camera->GetScreenRay(0.5, 0.5);
            for (auto box : m_boxes) {
                auto shape = box->GetComponent<CollisionShape>();
                float dist = ray.HitDistance(shape->GetWorldBoundingBox());
                if (std::isinf(dist)) {
                } else {
                    auto rb = box->GetComponent<RigidBody>();
                    rb->ApplyImpulse(ray.direction_ * 2000, box->WorldToLocal(ray.origin_ + ray.direction_ * dist));
                    m_text->SetText("Shot");
                }
            }
        }
        direction.Normalize();
        auto cm = rot * direction * smoothDeltaTime * move_speed;
        cm.y_ = rb->GetLinearVelocity().y_;
        rb->SetLinearVelocity(cm);
    }
};


URHO3D_DEFINE_APPLICATION_MAIN(Main)

