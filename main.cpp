#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <memory>
#include <array>
#include <list>
#include "particleEmitter.h"
std::string toString(long long int integer)
{
    std::ostringstream os;
    os << integer;
    return os.str();
}

float randomize(int min, int n){
return (std::rand()%n)+min;
}
struct rect{
sf::Vector2f pos;
sf::Vector2f size;

rect(const sf::Vector2f& p ={0.0f,0.0f}, const sf::Vector2f& s ={1.0f,1.0f}) :pos(p), size(s){}
bool contains(const sf::Vector2f& p) const{
	return(p.x>=pos.x && p.x<=pos.x+size.x && p.y>=pos.y && p.y <= pos.y+size.y);
}
bool contains(const rect& r) const{
return (r.pos.x>=pos.x && r.pos.x+r.size.x<=pos.x+size.x &&
 r.pos.y>=pos.y && r.pos.y+r.size.y<=pos.y+size.y);
}
bool overlaps(const rect& r) const{
return (pos.x < r.pos.x + r.size.x && pos.x+size.x>=r.pos.x && pos.y <r.pos.y+r.size.y && pos.y+size.y>=r.pos.y);
}
};
struct SomeObjectWithArea{
sf::Vector2f vPos;
sf::Vector2f vVel;
sf::Vector2f vSize;
sf::Color color;

};

constexpr size_t MAX_DEPTH=8;


template<typename T>
struct QuadTreeItemLocation{
typename std::list<std::pair<rect,T>>* container;
typename std::list<std::pair<rect,T>>::iterator iterator;
};

template <typename OBJECT_TYPE>
class DynamicQuadTree{
public:

DynamicQuadTree(const rect& size ={{0.0f,0.0f},{100.f,100.f}}, const size_t nDepth = 0){
m_depth=nDepth;
resize(size);
}

void resize(const rect& rArea){
clear();
m_rect=rArea;

//potomkowie mają połowe długości ojca
//kwadrat dzieli sie na 4 mniejsze
sf::Vector2f vChildSize=m_rect.size / 2.0f;
m_rChild={
rect(m_rect.pos, vChildSize), //1
rect({m_rect.pos.x+vChildSize.x,m_rect.pos.y},vChildSize), //2
rect({m_rect.pos.x,m_rect.pos.y+vChildSize.y},vChildSize), //3
rect({m_rect.pos.x+vChildSize.x,m_rect.pos.y+vChildSize.y}, vChildSize) //4
};
}

// funkcja clear czysci drzewo
void clear(){
	m_pItems.clear();
	for(int i=0; i<4; i++){
		if(m_pChild[i])	m_pChild[i]->clear();
		m_pChild[i].reset();
	}
}

size_t size() const{
	size_t nCount = m_pItems.size();
	for(int i=0; i<4; i++){
	if(m_pChild[i]) nCount+=m_pChild[i]->size();
	}
	return nCount;
}

QuadTreeItemLocation<OBJECT_TYPE> insert(const OBJECT_TYPE& item, const rect& itemSize){
for(int i=0; i<4; i++){
	//czy obiekt znajduje się w potomku?
	if(m_rChild[i].contains(itemSize)){
		//czy dotarliśmy do maksymalnej głębokości
		if(m_depth + 1 < MAX_DEPTH){
			//jeśli nie, to czy istnieje potomek?
			if(!m_pChild[i]){

			//nie, więc trzeba go stworzyć
			m_pChild[i]=std::make_shared<DynamicQuadTree<OBJECT_TYPE>>(m_rChild[i],m_depth);
			}
			//teraz napewno istnieje potomek, bo został utworzony
			//można wiec do niego coś dodać
			return m_pChild[i]->insert(item,itemSize);

		}
	}
}
	//nie zmiescil sie, wiec musi byc w aktualnej warstwie
	m_pItems.push_back({itemSize,item});
	return {&m_pItems,std::prev(m_pItems.end())};
}

	//zwraca obiekty w danym obszarze
std::list<OBJECT_TYPE> search(const rect& rArea) const{
	std::list<OBJECT_TYPE> listItems;
	search(rArea,listItems);
	return listItems;
}
void search(const rect& rArea, std::list<OBJECT_TYPE>& listItems) const{
//najpierw sprawdzamy obiekty, ktore naleza do listy
for(const auto& p : m_pItems){
	if(rArea.overlaps(p.first))
		listItems.push_back(p.second);
	}
	//potem rekurencyjnie sprawdz czy potomkowie naleza do listy
	for(int i=0; i<4; i++){
		if(m_pChild[i]){
			if(rArea.contains(m_rChild[i])) m_pChild[i]->items(listItems);

			else if(m_rChild[i].overlaps(rArea)) m_pChild[i]->search(rArea, listItems);
		}

	}
}

void items(std::list<OBJECT_TYPE>& listItems) const{
	for(const auto& p : m_pItems){
		listItems.push_back(p.second);
	}
	for(int i=0; i<4; i++){
		if(m_pChild[i]) m_pChild[i]->items(listItems);
	}
}
const rect& area(){
return m_rect;
}


protected:

//glebokosc drzewa
std::size_t m_depth = 0;

//pole drzewa czwórkowego
rect m_rect;

//cztery pola potomków drzewa
std::array<rect, 4> m_rChild{};

//potencjalni 4 potomkowie drzewa
std::array<std::shared_ptr<DynamicQuadTree<OBJECT_TYPE>>,4> m_pChild{};

//objekty aktualnie w tym drzewie
std::list<std::pair<rect, OBJECT_TYPE>> m_pItems;
};


template<typename T>
struct QuadTreeItem{
T item;
QuadTreeItemLocation<typename std::list<QuadTreeItem<T>>::iterator> pItem;
};

template<typename OBJECT_TYPE>
class DynamicQuadTreeContainer{
	using QuadTreeContainer = std::list<QuadTreeItem<OBJECT_TYPE>>;

	protected:
	QuadTreeContainer m_allItems;
	DynamicQuadTree<typename QuadTreeContainer::iterator> root;

	public:

	DynamicQuadTreeContainer(const rect& size={{0.f,0.f},{100.f,100.f}}, const size_t nDepth=0) : root(size){

	}

	void resize(const rect& rArea){
	root.resize(rArea);
	}
	size_t size() const{
	return m_allItems.size();
	}
	bool empty() const{
		return m_allItems.empty();
	}

	void clear(){
	root.clear();
	m_allItems.clear();
	}
	typename QuadTreeContainer::iterator begin(){
		return m_allItems.begin();
	}
	typename QuadTreeContainer::iterator end(){
		return m_allItems.end();
	}
	typename QuadTreeContainer::iterator cbegin(){
		return m_allItems.cbegin();
	}
	typename QuadTreeContainer::iterator cend(){
		return m_allItems.cend();
	}
	void insert(const OBJECT_TYPE& item, const rect& itemSize){

	    QuadTreeItem<OBJECT_TYPE> newItem;
	    newItem.item=item;


		m_allItems.push_back(newItem);
		m_allItems.back().pItem=root.insert(std::prev(m_allItems.end()),itemSize);
	}
	std::list<typename QuadTreeContainer::iterator> search(const rect& rArea){

		std::list<typename QuadTreeContainer::iterator> listItemPointers;
		root.search(rArea,listItemPointers);
		return listItemPointers;
	}
	void remove(typename QuadTreeContainer::iterator item){

	    item->pItem.container->erase(item->pItem.iterator);

	m_allItems.erase(item);
	}

	void relocate(typename QuadTreeContainer::iterator& item, const rect& itemsize){

	item->pItem.container->erase(item->pItem.iterator);

	item->pItem=root.insert(item,itemsize);
	}
};
int main(){
	srand( time( NULL ) );
	int bugsSquashed=0;
	float fArea=20000.f;
int searchSize=300;
const size_t SOMEOBJ_AMOUNT=10000;
const size_t BUGS_AMOUNT=500;
	sf::RenderWindow window(sf::VideoMode(1000,1000),"Implementacja drzewa czworkowego w c++ - KochacPolicje");
	//window.setVerticalSyncEnabled(true);
	sf::View view(sf::FloatRect(0.f,0.f,1000.f,1000.f));
	sf::View viewBackground(sf::FloatRect(0.f,0.f,1000.f,1000.f));
	window.setView(view);
	sf::Font font;
	if(!font.loadFromFile("font.ttf")){}
	sf::Text text;
	text.setFont(font);
	text.setFillColor(sf::Color::White);
	DynamicQuadTreeContainer<SomeObjectWithArea> treeObj;
	treeObj.resize(rect({0.f,0.f},{fArea,fArea}));
	for(int i=0; i<SOMEOBJ_AMOUNT; i++){
		SomeObjectWithArea ob;
		ob.vPos={randomize(0,(int)fArea),randomize(0,(int)fArea)};
		ob.vSize={randomize(100,300),randomize(100,300)};
		ob.color=sf::Color(randomize(0,30),randomize(100,255),randomize(0,30));
		ob.vVel={randomize(-2,4),randomize(-2,4)};
		treeObj.insert(ob,rect(ob.vPos,ob.vSize));
	}
	DynamicQuadTreeContainer<SomeObjectWithArea> treeBugs;
	treeBugs.resize(rect({0.f,0.f},{fArea,fArea}));
	for(int i=0; i<BUGS_AMOUNT; i++){
        SomeObjectWithArea bug;
        bug.vPos={randomize(0,(int)fArea),randomize(0,(int)fArea)};
		bug.vSize={randomize(120,240),randomize(120,240)};
		bug.color=sf::Color(randomize(0,255),randomize(0,255),randomize(0,255));
		bug.vVel={randomize(-2,4),randomize(-2,4)};
		treeBugs.insert(bug,rect(bug.vPos,bug.vSize));
	}
	sf::Clock clock;
	sf::Time elapsed;

		sf::RectangleShape background({1000.f,1000.f});
		background.setPosition({0.f,0.f});
		background.setFillColor(sf::Color(155, 90, 70));


        ParticleEmitter emitter(sf::seconds(3.f));
	while(window.isOpen()){
            window.clear();
	    window.setView(viewBackground);
            window.draw(background);

    window.setView(view);
            	 float scale=view.getSize().x/window.getSize().x;
            				sf::Vector2i localPosition=sf::Mouse::getPosition(window);
                sf::Vector2f worldPosition=window.mapPixelToCoords(localPosition);
                sf::Vector2i searchArea={searchSize,searchSize};
                sf::Vector2f fSearchArea={searchArea.x*scale,searchArea.y*scale};
                rect m(worldPosition, {searchArea.x*1.f,searchArea.y*1.f});
                sf::RectangleShape eraser({searchArea.x/scale,searchArea.y/scale});
                eraser.setPosition(localPosition.x*1.f,localPosition.y*1.f);
                eraser.setFillColor(sf::Color(200,200,200,100));

			rect rScreen={{view.getCenter().x-(view.getSize().x/2),view.getCenter().y-(view.getSize().y/2)}, {view.getSize().x,view.getSize().y}};
			elapsed=clock.restart();
			int amount=0;


	sf::Event event;
		while(window.pollEvent(event)){
		switch(event.type){
			case sf::Event::Closed:
				window.close();
			break;
			}
			if(sf::Keyboard::isKeyPressed(sf::Keyboard::Z)){
			view.zoom(0.95f);
			window.setView(view);
			}
			if(sf::Keyboard::isKeyPressed(sf::Keyboard::X)){
            if(scale<15.f){
			view.zoom(1.05f);
			window.setView(view);
			}
			}
			if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up)){
			view.move(0.f,-50.f);
			window.setView(view);
			}
			if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left)){
			view.move(-50.f,0.f);
			window.setView(view);
			}
			if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down)){
			view.move(0.f,50.f);
			window.setView(view);
			}
			if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right)){
			view.move(50.f,0.f);
			window.setView(view);
			}
           if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
            {
                window.setView(view);
                rect searchMouseArea({worldPosition.x-1.f,worldPosition.y-1.f}, {2.f,2.f});

                auto bushes=treeObj.search(searchMouseArea);
                if(bushes.empty()){

                }
                auto bugs=treeBugs.search(searchMouseArea);
                for(auto& bug : bugs){
                    emitter.addThing(bug->item.vPos);
                    treeBugs.remove(bug);
                    bugsSquashed++;
                }
            }
		}



			for(auto bug_it=treeBugs.begin(); bug_it!=treeBugs.end(); ++bug_it){
                    auto &b = bug_it->item;
                    b.vPos.x+=b.vVel.x*elapsed.asSeconds()*20;
                    b.vPos.y+=b.vVel.y*elapsed.asSeconds()*20;
                    if(b.vPos.x<0||b.vPos.x>fArea){
                        b.vVel.x*=-1.f;
                    }
                    if(b.vPos.y<0||b.vPos.y>fArea){
                        b.vVel.y*=-1.f;
                    }
                    treeBugs.relocate(bug_it,rect(b.vPos,b.vSize));
                    sf::RectangleShape thing(b.vSize);
                    thing.setFillColor(b.color);
                    thing.setPosition(b.vPos);
                    window.draw(thing);

			}
			for(auto& object : treeObj.search(rScreen)){


					sf::RectangleShape thing(object->item.vSize);
					thing.setFillColor(object->item.color);
					thing.setPosition(object->item.vPos);
					window.draw(thing);
					amount++;
			}

			if(bugsSquashed!=500)text.setString(toString(amount)+" z " + toString(SOMEOBJ_AMOUNT)+" \n w trakcie "+toString(elapsed.asMicroseconds())+" mikrosekund \n tryb: drzewo czworkowe \n"+"Rozdeptane robaki: "+toString(bugsSquashed)+" z "+toString(BUGS_AMOUNT));
            window.draw(emitter);
			window.setView(window.getDefaultView());
			window.draw(text);
			emitter.update(elapsed);
            if(bugsSquashed==500){
                text.setString("Gratulacje, wygrales! ");
            }
			window.draw(eraser);
			auto r=treeObj.search(m);
			for(auto& p : r){
                treeObj.remove(p);
			}
			window.display();
	}
return 0;
}
