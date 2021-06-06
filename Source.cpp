#include <iostream>
#include <fstream>
#include <vector>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <random>
#include <string>

using namespace std;

sf::RenderWindow*   window;
float               globalRT;
bool                globalGamestate;

//6 blocks with 3 animation frames

class UI;

UI*                 mainUi;

class Grid;

class Entity {
  protected :
  sf::Sprite  sprte;
  sf::Texture txtre;
  Grid* map;
  
  int pRx, pRy;

  float px = 0.5, py = 0.5; 
  int   pt = 0, ptc = 3;    //tick
  float ptt = 0.5, pte = 0; // tick time, tick excess
  float plt;                //last time
  
  int   height;
  
  public :
  void* item;

  int GetPx() { return ( int )( px - 0.5 ); }
  int GetPy() { return ( int )( py - 0.5 ); }
  
  int virtual PolyType () { return 0; }
  
  Entity ( int x, int y, const char* a1, int rx, int ry, int tc, int h, Grid* mp ) {
    item = 0;
    height = h; map = mp;
    pRx = rx; pRy = ry; ptc = tc;
    px += x; py += y; 
    plt = 0; 
    
    if( !txtre.loadFromFile( a1 ) ) { }
    sprte.setTexture( txtre );
    sprte.setTextureRect( sf::IntRect( rx * 32, ry * 32, 32, 32 ) );
  
    sprte.setOrigin ( sf::Vector2f( 16, 16 ) );
    sprte.setPosition ( sf::Vector2f( 32 * px, 32 * py ) );
  
  }
  
  Entity ( const Entity& a1 ) {
    height = a1.height;
    pRx = a1.pRx; pRy = a1.pRy; ptc = a1.ptc;
    px = a1.px; py = a1.py; plt = 0;
    txtre = sf::Texture( a1.txtre );
    sprte.setTexture( txtre );
    sprte.setTextureRect( sf::IntRect( pRx * 32, pRy * 32, 32, 32 ) );
    sprte.setOrigin ( sf::Vector2f( 16, 16 ) );
    sprte.setPosition ( sf::Vector2f( 32 * px, 32 * py ) );
    item = 0;
  }
  
  void Draw () {
    //sprte.setRotation( 30 );
    if ( ptc != 0 && globalRT - plt + pte >= ptt ) {
      pt = ( pt + 1 ) % ptc;
      plt = globalRT;
      sprte.setTextureRect( sf::IntRect( ( pRx + pt ) * 32, pRy * 32, 32, 32 ) );
    }
    window->draw( sprte );
  }

  bool CheckMove ( int x, int y );
  
  void virtual Move ( int x, int y ) {
    if ( !CheckMove( x, y ) || map == 0 ) { return; }
    px += x;
    py += y;
  }
  
  void SetPosition ( int x, int y ) {
    px = 0.5 + x;
    py = 0.5 + y;
  }
  
  void ForceDraw( int a1 = -1 ) {
    if( a1 == -1 ) { a1 = ( pt + 1 ) % ptc; }
    float delta = ptt;
    ptt = 0;
    pt = a1;
    Draw();
    ptt = delta;
  }
  
};

class Grid {
  int pH, pW;
 
  sf::Sprite  deltaS;

  int *intMap;

  int   sweepingTick;
  float tickLenght = 2;

  sf::Clock clock;
  float clockMargin;

  vector<Entity*> entities;

  public :
  sf::Texture spriteMap;

  Grid ( int h, int w, const char* smPath, const char* mapPath ) {
    pH = h; pW = w;
    
    ifstream in( mapPath );
    
    intMap = new int[ w * h ];
    
    for( int i = 0; i < w; i++ ) {
      for( int j = 0; j < h; j++ ) {
        in>>intMap[ pW * i + j ];
       // intMap[ pW * i + j ] = 0;
      }
    }
    
    if( !spriteMap.loadFromFile( smPath ) ) {
      return;
    }
    spriteMap.setRepeated ( true );
    
    deltaS.setTexture( spriteMap );

    clockMargin = 0;
  }

  void Draw() {
    for( int i = 0; i < pW; i++ ) {
      for ( int j = 0; j < pH; j++ ) {
        deltaS.setPosition( 32 * i, 32 * j );
        deltaS.setTextureRect ( ( sf::IntRect( ( ( sweepingTick + i + j ) * 0 % 3 ) * 32, intMap[ i * pW + j ] * 8, 32, 32 ) ) );
        window->draw( deltaS );
      } 
    }
    if( clock.getElapsedTime().asSeconds() + clockMargin > tickLenght ) {
      clockMargin = clock.getElapsedTime().asSeconds() - tickLenght;
      //sweepingTick = ( sweepingTick + 1 ) % 3;
      clock.restart();
    }
    for ( unsigned int i = 0; i < entities.size(); i++ ) {
      entities[ i ]->Draw();
    }
  }
  
  int GetBlock ( int x, int y ) {
    if( x * pW + y > pW * pH || x * pW + y < 0 ) return -1;
    return intMap[ x * pW + y ];
  }

  Entity* GetEntity( int x, int y ) {
    for ( unsigned int i = 0; i < entities.size(); i++ ) {
      if ( entities[ i ]->GetPx() == x && entities[ i ]->GetPy() == y ) {
        return entities[ i ];
      }
    }
    return 0;
  }

  Entity* GetEntity( int idx ) {
    return entities[ idx ];
  }

  void AddEntity ( Entity* ent ) {
    int x = ent->GetPx(), y = ent->GetPy();
    if ( GetBlock( x, y ) == 0 && GetEntity( x, y ) == 0 ) {
      entities.push_back( ent );
    }
  }
  
  void RemoveEntity( Entity* ent ) {
    for( vector<Entity*>::iterator i = entities.begin(); i != entities.end(); i++ ) {
      if( ent->GetPx() == (*i)->GetPx() && ent->GetPy() == (*i)->GetPy() ) {
        entities.erase( i );
        return;
      }
    }
  }
  
  void RemoveEntity( int idx ) {
    entities.erase( entities.begin() + idx );
  }
};

bool Entity::CheckMove ( int x, int y ) {
  if( map == 0 ) { return false; }
  //cout<<x<<" "<<y<<" "<<map->GetBlock( x, y )<<" "<<map->GetEntity( x, y )<<endl; cout.flush();
  if( map->GetBlock( x, y ) > height || map->GetEntity( x, y ) != 0 ) {
    return false;
  }
  return true;
} 

bool crt[3][3] = {
  1, 0, 0,
  0, 1, 0,
  1, 0, 1
};

class Creature;

class Pickable {
  public :
  virtual int PolyType() { return 0; }
};

class Item : public Entity {
  public :
  
  int PolyType() override { return 2; }
  
  int     cost;
  Pickable*   actual;
  
  Item ( int aCost, int px, int py, int rx, int ry, Pickable* aA ) : Entity ( px, py, "./items.png", rx, ry, 0, 0, 0 ) {
    cost = aCost;
    actual = 0;
    actual = aA;
  }
  
  Pickable* GetActual() {
    return actual;
  }
  
  void SetActual ( Pickable* a1 ) {
    actual = a1;
  }
};

class Weapon : public Pickable {
  
  int   baseDmg;
  int   failDmg;
  bool  iffInteract;
 
  float cooldown;
  float plt;  //last time
 
  int   chance;
  
  public :
  
  int PolyType() override { return 1; }
  
  Weapon ( int a1, bool a2, float a3, float a4 ) {
    baseDmg = a1;
    failDmg = a1 / 3;
    iffInteract = a2;
    cooldown = a3;
    chance = 100 - 100 * a4;
    plt = 0;
  }
  
  Weapon ( Weapon* a1 ) {
    baseDmg = a1->baseDmg;
    failDmg = baseDmg / 3;
    iffInteract = a1->iffInteract;
    cooldown = a1->cooldown;
    chance = a1->chance;
    plt = 0;
  }
  
  int Attack ( Creature* a1, Creature* a2 );
};

class Door : public Pickable {
 
  public :
 
  int PolyType() override { return 3; }
  
  int keyId;
  
  Door ( int kid ) { keyId = kid; }
  Door ( const Door& a1 ) { keyId = a1.keyId; }
  
  int GetId() {
    return keyId;
  }
  
};

class Key : public Pickable {
  
  int doorId;

  public :

  int PolyType() override { return 2; }
    
  Key ( int id ) {
    doorId = id;
  }
  
  Key ( const Key& a1 ) {
    doorId = a1.doorId;
  }
  
  bool CanOpenDoor ( Door a1 ) {
    if( a1.keyId == doorId || ( a1.keyId < 10 && doorId < 10 && a1.keyId < doorId ) ) {
      return true;
    }
    return false;
  }
  
  int GetId() {
    return doorId;
  }
  
};

class Creature : public Entity {
  int hp, mn, asc;  //association
  vector<Key> keyRing;
  
  public :
  
  int PolyType() override { return 1; }
  
  Weapon* weapon;
  
  void Move ( int x, int y ) override;
  
  Creature ( int x, int y, const char* a1, int rx, int ry, int tc, int h, Grid* mp, int ahp, int amn, int aasc ) 
   : Entity( x, y, a1, rx, ry, tc, h, mp ) {
    //cout<<"KEKS DE "<<PolyType()<< "\n";
    hp = ahp;
    mn = amn;
    asc = aasc;
    weapon = 0;
   }
   
  Creature ( const Creature& a1 ) : Entity( a1 ) {
    hp = a1.hp;
    mn = a1.mn;
    asc = a1.asc;
    if( a1.weapon != 0 ) {
      weapon = new Weapon( a1.weapon );
    } else {
      weapon = 0;
    }
  }
  
  bool GetIFF ( Creature& a1 ) {
    int d1 = asc, d2 = a1.GetAssociation ();
    if( d2 < d1 ) { d1 = d2; d2 = asc; }
    return crt[ d1 ][ d2 ];
  }
  
  int GetAssociation () {
    return asc;
  }
  
  void Damage( int a1 ) {
    hp -= a1;
    if( hp < 0 ) {
      map->RemoveEntity ( this );
    }
  }
  
  bool Take ( Pickable* a1 );
  
};

int Weapon::Attack ( Creature* a1, Creature* a2 ) {
  int omega = baseDmg;
  if ( globalRT - plt < cooldown ) {
    omega = failDmg;
  } else {
    plt = globalRT;
  }
  if( rand() % 100 < chance ) { omega = failDmg; }
  a2 -> Damage ( omega / ( a1->GetIFF( *a2 ) || iffInteract ? 1 : 2 ) );      
  return omega;
}

const float mainVR = 2; //visual ratio
  
class UI {
  
  sf::Texture tx;
  sf::Font    ft;
  sf::Sprite  sp;
  sf::Text    sLabel;
  
  int         wS;
  
  struct Button {
    char* label;
    void (*onClick)( void );
    int py;
    
    Button( const char* a1, void (*oc)(void), int apy ) {
      label = new char( 32 );
      int lg = 0;
  
      for( lg = 0; a1[ lg ] != 0; lg++ ) { label[ lg ] = a1[ lg ]; }
      label[ lg ] = 0;
  
      onClick = oc;
  
      selected = false;
      
      py = apy;
    }
    
    bool selected;
    
    void Draw ( UI core ) {
      core.sp.setPosition ( 5920, 6050 - 20 * py + 4 );
      core.sLabel.setPosition( 5930, 6053 - 20 * py + 4 );
      if( selected ) { core.sp.setTextureRect( sf::IntRect ( 32, 0, 32, 32 ) ); }
      window->draw( core.sp );
      if( selected ) { core.sp.setTextureRect( sf::IntRect ( 0, 0, 32, 32 ) ); }
      core.sLabel.setString( label );
      window->draw( core.sLabel );
    }
  
    void ChS( bool a1 ) { selected = a1; }
    
    void Enter () { if( onClick != 0 ) onClick(); }
    
  };
  
  vector<Button> buttons;
  bool active, latch, udLatch;
  int internal;
  
  public :
  
  int flag;
  
  bool GetActive() { return active; }
  
  UI ( vector<const char*> a1, vector<void(*)()> a2 ) {
    if ( a1.size() != a2.size() ) { return; }
    sp = sf::Sprite();
    if( !tx.loadFromFile("./ui.png") ) { return; }
    if( !ft.loadFromFile("./OM.ttf") ) { return; }
    sLabel.setFont( ft );
    sLabel.setCharacterSize( 20 );
    sLabel.setScale( 0.5, 0.5 );
    sp.setTexture ( tx );
    sp.setTextureRect( sf::IntRect( 0, 0, 32, 32 ) );
    sp.setPosition ( 6000 + 20, 6000 + 40 );
    sp.setScale ( 2, 0.5 );
    for( unsigned int i = 0; i < a1.size(); i++ ) {
      buttons.push_back ( Button( a1[ i ], a2[ i ], i ) );
    }
    internal = 0;
    target.setCenter ( 6000, 6000 );
    target.setSize ( 200, 200 );
    flag = 0;
  }
  
  sf::View delta;
  sf::View target;
  
  float deltaMessage;
  float lt;
  
  bool OnTick() {
    
    if( sf::Keyboard::isKeyPressed( sf::Keyboard::Escape ) ) {
      if ( latch ) { active = !active;
        if( active ) {
          delta = window->getView();
          window->setView( target );
        } else {
          window->setView( delta );
        }
      }
      latch = false;
    } else {
      latch = true;
    }
    
    if( active ) {
    
      sLabel.setCharacterSize( 20 );
      sLabel.setScale( 0.5, 0.5 );
      sp.setScale ( 2, 0.5 );
    
      if( sf::Keyboard::isKeyPressed( sf::Keyboard::Down )  || sf::Keyboard::isKeyPressed( sf::Keyboard::Up ) 
       || sf::Keyboard::isKeyPressed( sf::Keyboard::Enter ) || sf::Keyboard::isKeyPressed( sf::Keyboard::A ) ) {
        if( udLatch ) {
          if ( sf::Keyboard::isKeyPressed( sf::Keyboard::Down ) ) {
            buttons[ internal ].ChS( false );
            internal--;
          }
          if ( sf::Keyboard::isKeyPressed( sf::Keyboard::Up ) ) {
            buttons[ internal ].ChS( false );
            internal++;
          } 
          udLatch = false;
          internal = internal % buttons.size();
          buttons[ internal ].ChS( true );
          
          if ( sf::Keyboard::isKeyPressed( sf::Keyboard::Enter ) || sf::Keyboard::isKeyPressed( sf::Keyboard::A ) ) {
            buttons [ internal ].Enter();
          }
        }
      } else {
        udLatch = true;
      }
      
      for( unsigned int i = 0; i < buttons.size(); i++ ) {
        buttons[ i ].Draw( *this );
      }
    }
    return active;
  }
  
  void Message ( const char* a1, float time ) {
    sLabel.setString( a1 );
    deltaMessage = time;
    sp.setPosition ( 32 * 3, 270 );
    sp.setScale( 4, 2 );
    sLabel.setPosition ( 32 * 3.5, 275 );
    lt = globalRT;
  }
  
  void Draw() {
    if ( deltaMessage > 0 ) {
      deltaMessage -= globalRT - lt; 
      lt = globalRT;
      window->draw( sp );
      window->draw( sLabel );
      if ( deltaMessage < 0 ) {
        flag = 0;
      } else {
        flag = 1;
      }
    }
  }
  
};

void Creature::Move ( int x, int y ) {
  if ( CheckMove( GetPx() + x, GetPy() + y ) ) {
    px += x;
    py += y;
    sprte.setPosition( px * 32, py * 32 );
    //cout<<px<<" "<<py<<" kek\n"; cout.flush();
  } else {
    void* a1 = map->GetEntity( GetPx() + x, GetPy() + y ); if( a1 == 0 ) { return; }
    
    if ( static_cast<Entity*>(a1)->PolyType() == 1 && weapon != 0 ) {
      string delta = "Dealt ";
      delta += to_string( weapon->Attack ( this, static_cast<Creature*>( a1 ) ) );
      delta += " damage!";
      mainUi->Message( delta.c_str(), 2 );
    }
    
    if ( static_cast<Entity*>(a1)->PolyType() == 2 ) {
      bool deltaTaken = Take( static_cast<Item*>(a1)->actual );
      if( deltaTaken ) {
        map->RemoveEntity ( (Entity*)a1 );
      }
    }
  }
}

bool Creature::Take ( Pickable* a1 ) {
  //Pickable* deltaPk = static_cast<Pickable*>( static_cast<Item*>(a1)->actual );
  if( a1 == 0 ) { return false; }
  
  if ( a1->PolyType() == 1 ) {
    weapon = new Weapon( static_cast<Weapon*>( a1 ) );
    mainUi->Message( "Got a new weapon!", 2 );
    return true;
  }

  if ( a1->PolyType() == 2 ) {
    keyRing.push_back( *static_cast<Key*> ( a1 ) );
    string delta = "Got a ";
    if( keyRing[ keyRing.size() - 1 ].GetId() < 10 ) {
      delta += "level ";
      delta += to_string ( keyRing[ keyRing.size() - 1 ].GetId() );
      delta += "\nkeycard!";
    } else {
      delta += " keycard with the id ";
      delta += to_string ( keyRing[ keyRing.size() - 1 ].GetId() );
    }
    mainUi->Message( delta.c_str(), 2 );
    return true;
  } 
  
  if ( a1->PolyType() == 3 ) {
    string delta;
    Door* deltaD = static_cast<Door*>( a1 );
    for( vector<Key>::iterator i = keyRing.begin(); i != keyRing.end(); i++ ) {
      if ( (*i).CanOpenDoor( *deltaD ) ) {
        delta = "Door open! Used\nkeycard ";
        delta += to_string( (*i).GetId() );
        delta += " !";
        mainUi->Message( delta.c_str(), 2 );
        keyRing.erase( i );
        return true;
      }
    }
    delta = "Door Closed! Need\nkeycard ";
    delta += to_string( deltaD->GetId() );
    delta += " !";
    mainUi->Message( delta.c_str(), 2 );
  }
  
  return false;
}

void TEST() {
  globalGamestate = false;
}
  
int main () {
  globalGamestate = true;
  
  sf::RenderWindow wnd(sf::VideoMode(1366, 768), "CORE");
  window = &wnd;
  
  sf::View view(sf::FloatRect(0, 0, 400, 400 ));
  view.setCenter( 175, 175 );
  //view.setRotation( 30 );
  window->setView( view );
  
  Grid core = Grid( 10, 10, "./test.png", "./01.lvl" );
  UI pause ( {"Exit","Continue"}, {TEST, 0} );
  mainUi = &pause;
  
  core.AddEntity ( new Creature ( 5, 5, "./test.png", 1, 3, 3, 3, &core, 10, 10, 0 ) );
  
  core.AddEntity ( new Item( 0, 2, 2, 0, 0, new Weapon( 100, true, 1, 0.5 ) ) );
  core.AddEntity ( new Item( 0, 2, 3, 1, 0, new Door ( 1 ) ) );
  core.AddEntity ( new Item( 0, 2, 4, 1, 0, new Door ( 12 ) ) );
  core.AddEntity ( new Item( 0, 2, 6, 2, 0, new Key ( 1 ) ) );
  
  core.AddEntity ( new Creature ( 7, 7, "./test.png", 1, 3, 3, 3, &core, 10, 10, 1 ) );
  
  sf::Clock clk;
  
  bool keyDown = false;
  int dx = 0, dy = 0;
  
  pause.Message( "HENLO", 2 );
  
  while ( window->isOpen() && globalGamestate ) {
    globalRT = clk.getElapsedTime().asSeconds();
    
    window->clear();
    
    if( !pause.OnTick() ) {
    
      sf::Event event;
      while (window->pollEvent(event))
      {
        
        
        if (event.type == sf::Event::Closed) {
            window->close();
        }
        
        if (event.type == sf::Event::Resized) {
          float correctR = ( event.size.height * mainVR ); 
        
          if ( correctR < event.size.width ) {
        
            view.setViewport ( sf::FloatRect ( ( event.size.width - correctR ) / ( 2.0f * event.size.width ), 0, correctR / event.size.width, 1 ) );
        
          } else {
        
            correctR = ( event.size.width / mainVR );
            view.setViewport ( sf::FloatRect( 0, ( event.size.height - correctR ) / ( 2 * event.size.height ), 1, correctR / event.size.height ) );
        
          }
        
          window->setView( view );
        
        }
        
        if( event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased ) { 
          dx = 0; dy = 0;
          if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) ) {
            dx--; 
          }
          if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) ) {
            dx++; 
          }
          if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) ) {
            dy--;
          }
          if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) ) {
            dy++;
          };
        }
      }
  
      if( dx == 0 && dy == 0 ) {
        keyDown = false;
      } else {
        if ( !keyDown ) {
          core.GetEntity ( 0 )->Move( dx, dy );
          keyDown = true;
        }
      }
    }
    
    if( pause.flag == 1 || !pause.GetActive() ) { core.Draw(); }
  
    pause.Draw();
    window->display();
    
    //cout.flush();
  }

}
