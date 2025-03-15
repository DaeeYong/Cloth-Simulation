#include <iostream>
#include <JGL/JGL_Window.hpp>
#include "AnimView.hpp"
#include <glm/gtx/quaternion.hpp>

using namespace glm;

struct Particle {
	vec3 x;
	vec3 v;
	vec3 f;
	float m;
	Particle( float mass, const vec3& position, const vec3& velocity=vec3(0) )
		: x( position ), v( velocity ), m( mass ) {}
	void clearForce() {
		f = vec3(0);
	}
	void add( const vec3& force ) {
		f+=force;
	}
	void update( float deltaT ) {
		v += f / m * deltaT;
		x += v * deltaT;
	}
	void draw() {
		drawSphere( x, 0.8, glm::vec4(0.933, 0.509, 0.933, 1));
	}
};

struct Spring {
	Particle& a;
	Particle& b;
	float restLength;
	float k = 20.f;
	float kd = 0.001f;
	Spring( Particle& x, Particle& y ) : a(x), b(y), restLength( length(x.x-y.x)) {
	}
	void addForce() {
		vec3 dx = a.x - b.x; //differ of position
		vec3 dv = a.v - b.v; //differ of velocity
		vec3 f = (k * (length(dx) - restLength) + kd * (glm::dot(dx, dv))) * normalize(dx); //direction
		a.add(-f);
		b.add(f);
	}
	void draw() {
		drawCylinder( a.x, b.x, 0.35, glm::vec4(0.8, 1, 0.8, 1) );
	}
};

struct Shpere {
	vec3 center;
	float radius;
	float alpha = 0.6;
	Shpere(const vec3& position, const float r) : center(position), radius(r) {}
	
	void draw() {
		drawSphere(center, radius, vec4(1, 0.647, 0, 1));
	}
	void resolveCollision(Particle& particle) {
		vec3 N = normalize(particle.x - center);
		float dist = length(particle.x - center) - radius;

		const float epsilon = 0.001;
		if (dist < epsilon) {
			float v = dot(N, particle.v);
			if (v < -epsilon) {
				vec3 vN = v * N;
				vec3 vT = particle.v - vN;
				particle.v = vT - alpha * vN;
			}
			else if (dist < epsilon) {
				vec3 vN = v * N;
				vec3 vT = particle.v - vN;
				particle.v = vT;

			}
			particle.x += -dist * N;
		}
	}
};

struct Plane {
	vec3 N;
	vec3 p;
	float alpha = 0.6;
	Plane( const vec3& position, const vec3& normal ): N(normal), p(position){}
	void draw() {
		drawQuad(p,N,{1000,1000}, vec4(0.9, 0.9, 0.8, 1));
	}
	void resolveCollision( Particle& particle ) {
		float dist_sin = glm::dot(N, particle.x - p);
		const float epsilon = 0.001;
		if (dist_sin < epsilon) {
			float v = dot(N, particle.v);
			//�������� ���� ���
			if (v < -epsilon) {
				vec3 vN = v * N;
				vec3 vT = particle.v - vN;
				particle.v = vT - alpha * vN;
			}
			else if (dist_sin < epsilon) {
				vec3 vN = v * N;
				vec3 vT = particle.v - vN;
				particle.v = vT;

			}
			particle.x += -dist_sin * N;
		}
	}
};

float randf() {
	return rand()/(float)RAND_MAX;
}

bool fix0 = true, fix1 = true;

void keyFunc(int key) {
	if( key == '1' )
		fix0=!fix0;
	if( key == '2' )
		fix1=!fix1;
}

const vec3 G ( 0, -980.f, 0 );
const float k_drag = 0.01f;
std::vector<Particle> particles;
std::vector<Spring> springs;
Shpere sphere({ 0,25,-5 }, 25);
Plane flooring( {0,0,0}, {0,1,0} );
const int count = 20;

void init() {
	particles.clear();
	springs.clear();
	fix0 = true, fix1 = true;
	const int particle_dist = 2;
	const float seed = 0.1;
	
	for (int y = 0; y < count; y++) {
		for (int x = 0; x < count; x++) {
			particles.emplace_back(0.001f, vec3(particle_dist*x - 5, particle_dist*y+52, randf() * seed));
		}
	}
	//connect spring
	for (int y = 0; y < count-1; y++) {
		for (int x = 0; x < count; x++) {
			springs.emplace_back(particles[y * 20 + x], particles[(y + 1) * 20 + x]);
		}
	}
	for (int y = 0; y < count; y++) {
		for (int x = 0; x < count-1; x++) {
			springs.emplace_back(particles[y * 20 + x], particles[y*20 + x + 1]);
		}
	}
	for (int y = 0; y < count-1; y++) {
		for (int x = 0; x < count-1; x++) {
			springs.emplace_back(particles[y * 20 + x], particles[(y+1) * 20 + x + 1]);
		}
	}
	for (int y = 0; y < count-1; y++) {
		for (int x = 0; x < count-1; x++) {
			springs.emplace_back(particles[(y+1) * 20 + x], particles[y * 20 + x + 1]);
		}
	}
}

void frame( float dt ) {
	const int steps = 200;
	for( int i = 0; i<steps; i++ )
	{
		vec3 p0 = particles[(count - 1) * count].x;
		vec3 p1 = particles[(count * count) - 1].x;

		for (auto& p : particles) p.clearForce();
		for (auto& s : springs) s.addForce();
		for (auto& p : particles) p.add(p.m * G); //gravity
		for (auto& p : particles) p.add(-k_drag * p.v); //viscous drag
		for (auto& p : particles) p.update(dt/steps);
		for (auto& p : particles) flooring.resolveCollision(p);
		for (auto& p : particles) sphere.resolveCollision(p);
		
		if(fix0 == true){
			particles[(count - 1) * count].x = p0;
			particles[(count - 1) * count].v = vec3(0, 0, 0);

		}
		if (fix1 == true) {
			particles[(count * count) - 1].x = p1;
			particles[(count * count) - 1].v = vec3(0, 0, 0);
		}
	}
}

void render() {
	for( auto& p : particles ) p.draw();
	for( auto& s : springs )   s.draw();
	flooring.draw();
	sphere.draw();
}

int main(int argc, const char * argv[]) {
	JGL::Window* window = new JGL::Window(800,600,"Cloth simulation");
	window->alignment(JGL::ALIGN_ALL);
	AnimView* animView = new AnimView(0,0,800,600);
	animView->renderFunction = render;
	animView->frameFunction = frame;
	animView->initFunction = init;
	animView->keyFunction = keyFunc;
	init();
	window->show();
	JGL::_JGL::run();
	return 0;
}
