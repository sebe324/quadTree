#ifndef PARTICLEEMITTER_H
#define PARTICLEEMITTER_H

#include <SFML/Graphics.hpp>

struct Particle{
sf::RectangleShape shape;
sf::Vector2f vVel;
};
struct Thing{
std::vector<Particle> particles;
sf::Time timeLeft;
};
class ParticleEmitter : public sf::Drawable{
    public:

sf::Time maxTime;
std::vector<Thing> things;
ParticleEmitter(sf::Time mt);
void update(sf::Time elapsed);
void addThing(sf::Vector2f v2f);

    private:
        virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
        float randomize(int min, int n);
};
#endif // PARTICLEEMITTER_H
