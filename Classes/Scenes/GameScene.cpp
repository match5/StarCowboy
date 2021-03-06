#include "GameScene.h"
#include "GameWorld.h"
#include "GameObjectManager.h"

//fortest
#include "MathUtils.h"
#include "GameObject.h"
#include "ComShipBody.h"
#include "ComShipController.h"
#include "ComPhysicsEntity.h"
#include "ComEngine.h"
#include "ComWeaponSystem.h"
#include "ComParticleEmiter.h"
#include "ComAiEnemyController.h"
#include "ComHitPoint.h"
#include "Cannon.h"
#include "Laser.h"
#include "MissileLauncher.h"
#include "GameObjectFactory.h"

#include "Danmaku_1.h"
#include "Danmaku_2.h"
#include "Danmaku_3.h"

USING_NS_CC;

Scene* GameScene::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = GameScene::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool GameScene::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    addChild(GameWorld::getInstance()->getGameLayer());
    addChild(GameWorld::getInstance()->getUiLayer());
    
    GameObject *playerShip = GameWorld::getInstance()->getObjectManager()->createObject("player").get();
    playerShip->setTagBits(Tag::getBit("player") | Tag::getBit("physics"));
    playerShip->addComponent(ComPhysicsEntity::create(3, 500), "entity");
    playerShip->addComponent(ComShipBody::create(CREATE_SPRITE("ship.png")), "body");
    playerShip->addComponent(ComParticleEmiter::create(cc::ParticleSystemQuad::create("particles/cosmic_particle.plist")));
    playerShip->addComponent(ComParticleEmiter::create(cc::ParticleSystemQuad::create("particles/ship_wake_0.plist")), "wake_emiter");
    playerShip->addComponent(ComShipController::create(), "controller");
    playerShip->addComponent(ComEngine::create(50000, 50), "engine");
    playerShip->addComponent(ComWeaponSystem::create()
                             ->setTargetMask(Tag::getBit("enemy"))
                             ->addWeapon(Cannon::create()
                                         ->setDamage(40)
                                         ->setCoolDown(0.15f)
                                         ->setRange(200)
                                         ->setErrorAngle(2.5)
                                         ->addHitTestMask(Tag::getBit("rock")))
                             ->addWeapon(MissileLauncher::create()
                                         ->setDamage(50)
                                         ->setMaxAmmoNum(8)
                                         ->setMissileSpeed(100)
                                         ->setCoolDown(0.25)
                                         ->setSearchAngular(60)
                                         ->setAngularSpeed(180)
                                         ->addHitTestMask(Tag::getBit("rock")))
                             ->addWeapon(Laser::create(CREATE_SPRITE("laser.png"))
                                         ->setDPS(200)
                                         ->addHitTestMask(Tag::getBit("rock")))
, "weapon");
    playerShip->addComponent(ComHitPoint::create(200));
    playerShip->awake();
    
    for (int i = 0; i < 50; ++i) {
        GameObjectFactory::createRandomRock(cc::Vec2(random(-200.f, 200.f), random(-200.f, 200.f)))->awake();
    }
    
    int maxEnemyNum = 10;
    int waveEnemyNum = 2;
    std::function<void(float)> addEnemy = [=](float){
        GameObject *player = GameObjectManager::getInstance()->getObjectHandle("player").get();
        if (player == nullptr || !player->isActive()) {
            this->unschedule("add_enemy");
            return;
        }
        int num = 0;
        GameWorld::getInstance()->getObjectManager()->enumerateObject(Tag::getBit("enemy"), [&num](GameObject*){
            ++num;
        });
        num = std::min(maxEnemyNum - num, waveEnemyNum);
        for (int i = 0; i < num; ++i) {
            GameObject *enemy = GameWorld::getInstance()->getObjectManager()->createObject().get();
            enemy->addComponent(ComPhysicsEntity::create(3, 200), "entity");
            enemy->setTagBits(Tag::getBit("enemy") | Tag::getBit("physics"));
            enemy->addComponent(ComShipBody::create(CREATE_SPRITE("enemy.png")), "body");
            enemy->addComponent(ComParticleEmiter::create(cc::ParticleSystemQuad::create("particles/ship_wake_1.plist")), "wake_emiter");
            cc::Vec2 locate(random(-100, 100), random(-100, 100));
            enemy->getComponent<ComPhysicsEntity>("entity")->setLocation(player->getComponent<ComPhysicsEntity>("entity")->getLocation() + locate.getNormalized() * random(300, 500));
            enemy->addComponent(ComEngine::create(2000, 45), "engine");
            if (random(0, 100) >= 50) {
                enemy->addComponent(ComWeaponSystem::create()
                                    ->setTargetMask(Tag::getBit("player"))
                                    ->addWeapon(MissileLauncher::create()
                                                ->setDamage(10)
                                                ->setMaxAmmoNum(2)
                                                ->setReloadTime(3)
                                                ->setMissileSpeed(75)
                                                ->setCoolDown(0.25)
                                                ->setSearchAngular(30)
                                                ->setAngularSpeed(90)
                                                ->addHitTestMask(Tag::getBit("rock"))), "weapon");
            } else {
                enemy->addComponent(ComWeaponSystem::create()
                                    ->setTargetMask(Tag::getBit("player"))
                                    ->addWeapon(Danmaku_1::create()
                                                ->setDamage(10)
                                                ->setBulletSpeed(100)
                                                ->setCoolDown(2.0)
                                                ->setRange(150)
                                                ->setErrorAngle(5)
                                                ->setBulletNum(3)
                                                ->setShotInterval(0.1)
                                                ->addHitTestMask(Tag::getBit("rock"))), "weapon");
            }
            
            enemy->addComponent(ComAiEnemyController::create());
            enemy->addComponent(ComHitPoint::create(100));
            enemy->awake();
        }
    };
    addEnemy(0);
    schedule(addEnemy, 10, "add_enemy");
    
    std::function<void(float)> checkRestartGame = [](float) {
        GameObject* player =  GameWorld::getInstance()->getObjectManager()->getObjectHandle("player").get();
        if (!player || !player->isActive()) {
            GameObjectManager::destroy();
            GameWorld::destroy();
            cc::Director::getInstance()->replaceScene(GameScene::createScene());
        }
    };
    schedule(checkRestartGame, 10, "check_restart_game");

    scheduleUpdate();
    
    return true;
}

void GameScene::update(float dt) {
    static float fixedDeltaTime = 1 / 60.f;
    GameWorld::getInstance()->update(fixedDeltaTime);
}
