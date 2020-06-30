#ifndef ENTITY_H
#define ENTITY_H
#pragma once 

#include <SFML/Graphics.hpp>

#include <memory>
#include <vector>

class IEntityObjectBuilder;
class IGlobalMovementComponent;
class EntityObject;

struct Input;
struct EntityUpdate;

typedef std::map<std::string, std::shared_ptr<EntityObject>> EntityManifest;

class Entity {

public:
	Entity() = default;
	Entity(
		std::shared_ptr<IEntityObjectBuilder> entityBuilder,
		std::shared_ptr<IGlobalMovementComponent> globalMovementComponent);
	virtual ~Entity() = default;

	void AddObject(std::string name, std::shared_ptr<EntityObject> object);
	void RemoveObject(std::string name);
	std::shared_ptr<EntityObject> GetObject(std::string name) const;

	virtual void Update(float dt) const = 0;
	virtual void Draw(sf::RenderTarget& target, float interp) const = 0;

	bool DetectCollision(sf::FloatRect hitbox) const;

protected:
	void UpdateObjects(std::map<std::string, EntityUpdate> update, float dt) const;
	void DrawObjects(sf::RenderTarget& target, sf::Vector2f interPosition) const;

	EntityManifest objects;
	std::shared_ptr<IEntityObjectBuilder> entityBuilder;
	std::shared_ptr<IGlobalMovementComponent> globalMovementComponent;

private:

};

#endif //ENTITY_H