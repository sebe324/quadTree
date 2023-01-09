#include "particleEmitter.h"

ParticleEmitter::ParticleEmitter(sf::Time mt){
maxTime=mt;
}
void ParticleEmitter::addThing(sf::Vector2f v2f){
Thing t;
for(int i=0; i<10; i++){
Particle p;
sf::RectangleShape rs({50.f,50.f});
rs.setPosition(v2f);
p.vVel={randomize(-100.f,200.f),randomize(-100.f,200.f)};
rs.setFillColor(sf::Color(randomize(100,255),randomize(0,50),randomize(0,50)));
t.timeLeft=maxTime;
p.shape=rs;

t.particles.push_back(p);
}
things.push_back(t);
}
void ParticleEmitter::update(sf::Time elapsed){
for(int i=0; i<things.size(); i++){
    things[i].timeLeft-=elapsed;
    for(int j=0; j<10; j++){
    things[i].particles[j].shape.move(things[i].particles[j].vVel*elapsed.asSeconds());
    }
    if(things[i].timeLeft<=sf::Time::Zero){
        things.erase(things.begin()+i);
    }
}
}

void ParticleEmitter::draw(sf::RenderTarget& target, sf::RenderStates states) const{
for(int i=0; i<things.size(); i++){
        for(int j=0; j<10; j++){
    target.draw(things[i].particles[j].shape,states);
}
}
}

float ParticleEmitter::randomize(int min, int n){
return (std::rand()%n)+min;
}
