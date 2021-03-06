
#ifndef __GameComponent_h__
#define __GameComponent_h__

#include "cocos.h"

#include "GameObject.h"
#include "GameWorld.h"
#include "GameMessage.h"
#include "Counter.h"

#include <string>

class ComponentBace : public GameMessageHandler {
    
    friend class GameObject;
    
public:
    
    ComponentBace() : _owner(nullptr) {
    }
    
    virtual ~ComponentBace() = 0;
    
    const std::string& getName() const { return _name; }
    
    void setName(const std::string& name) {
        _name = name;
    }
    
    GameObject* getOwner() {
        return _owner;
    }
    
    GameWorld* getWorld() const;
    
    void setEnabled(const bool b) {
        _isEnabled = b;
    }
    
    const bool isEnabled() const {
        return _isEnabled;
    }
    
protected:
    
    virtual void onLoad() {}
    virtual void onUnload() {}
    
    virtual void start() {}
    virtual void update(float td) {}
    
    virtual void onOwnerAwake() {}
    virtual void onOwnerSleep() {}
    virtual void onOwnerDead() {}
    
    void setOwner(GameObject* obj) {
        _owner = obj;
    }
    
    bool isStarted() {
        return _isStarted;
    }
    
    GameObject* _owner = nullptr;
    
    std::string _name;
    
    bool        _isEnabled = true;
    
    void        setUpdateInterval(float interval) {
        _updateTimer.reset(interval);
    }
    
private:
    
    void        doUpdate(float dt);
    
    bool        _isStarted = false;
    Counter<float>       _updateTimer;
};

#endif
