#include <iostream>
#include <queue>
#include <cmath>
#include <cstdlib>
#include <set>
#include <vector>
#include <functional>
#include <numeric>
#include <bitset>
#include <cassert>
#include <map>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>   
#include <SDL.h>
#include <SDL_image.h>

uint64_t getInitState() {
	return 0xF973C401AB8D6E25ULL;
	//return 0x0123456789ABFCDEULL;
}

bool isFinalState(uint64_t state) {
	return state == 0x0123456789ABCDEFULL;
}

int getVal(uint64_t state, unsigned pos) {
	return (state >> ((15 - pos) * 4)) & 15u;
}

bool isSolveable(uint64_t state) {
	std::vector<int> v = { 0,1,2,3,7,6,5,4,8,9,10,11,15,14,13,12 };
	std::transform(v.begin(), v.end(), v.begin(), [state](int x) {return getVal(state, x); });
	v.erase(std::find(v.begin(), v.end(), 15));
	int sum = 0;
	for (int i = 0; i < 15; ++i) {
		int cnt = 0;
		for (int j = 0; j < i; ++j) {
			if (v[i] > v[j]) cnt++;
		}
		sum += cnt;
	}
	return !(sum % 2);
}


bool isValidState(uint64_t state) {
	std::vector<int> b(16,0);
	for (int i = 0; i < 16; ++i)
	{
			b[getVal(state, i)]++;
	}
	for (int i = 0; i < 16; i++)
	{
		if (b[i] > 1) { std::cout << i << " tobb mint 1x\n"; return false; }
		if (b[i] < 1) { std::cout << i << " kevesebb mint 1x\n"; return false; }
	}
	return true;
}
uint16_t getNeighbours(unsigned pos, int& num) {
	uint16_t ret = 0;
	num = 0;
	//left

	if (pos % 4 != 0) {
		ret <<= 4;
		ret += pos - 1;
		num++;
	}
	//up
	if (pos > 3) {
		ret <<= 4;
		ret += pos - 4;
		num++;
	}
	//right		
	if (pos % 4 != 3){
		ret <<= 4;
		ret += pos + 1;

		num++;
	}
	//down
	if (pos < 12) {
		ret <<= 4;
		ret += pos + 4;
		num++;
	}
	return ret;
}

inline int getRowNum(unsigned pos) {
	return pos / 4;
}

inline int getColNum(unsigned pos) {
	return pos % 4;
}


unsigned heuristic(uint64_t state) {
	//return the sum of the manhattan distance of the tiles from their final position
	unsigned sum = 0;
	for (int i = 15; i >= 0; i--)
	{
		unsigned num = state & 15u;
		sum += std::abs(getRowNum(num) - getRowNum(i)) + std::abs(getColNum(num) - getColNum(i));
		//std::cout << i << " " << num << " "<< std::abs(getRowNum(num) - getRowNum(i)) << " " << std::abs(getColNum(num) - getColNum(i)) << "\n";
		state >>= 4;
	}
	return sum;
}

int getEmptyPos(uint64_t state) {
	//return the sum of the manhattan distance of the tiles from their final position
	for (int i = 15; i >= 0; i--)
	{
		if ((state & 15u) == 15u) return i;
		state >>= 4;
	}
}
std::map<uint64_t, unsigned> level;
struct less_heur {
	bool operator()(uint64_t a, uint64_t b) { return heuristic(a)*20 + level[a] > heuristic(b)*20 + level[b]; }
};


uint64_t getNextState(int emptyPos, int fromPos, uint64_t state) {
	// 0 out emptyPos

	//~(15 << (15 - emptyPos));
	state &= ~(((uint64_t)(15-getVal(state,fromPos))) << ((15-emptyPos)*4));
	state |= (15ull << ((15-fromPos)*4));
	return state;
}

void printState(uint64_t state, std::ostream& os = std::cout) {
	for (int i = 0; i < 16; ++i)
	{
		os << getVal(state, i) << ",";
		if (i % 4 == 3) os << std::endl;
	}
	os << "=========\n";
}

bool solve(uint64_t state, std::map<uint64_t, uint64_t>& parent) {

	if (!isSolveable(state)) {
		std::cout << "Nem megoldhato\n";
		return false;
	}
	//A*
	std::priority_queue<uint64_t, std::vector<uint64_t>, less_heur> Q;
	std::set<uint64_t> visitedStates;
	level[state] = 0;
	Q.push(state);
	int num;
	while (!Q.empty()) {
		uint64_t actState = Q.top();
		//printState(actState);
		visitedStates.insert(actState);
		//assert(isValidState(actState));
		if (isFinalState(actState))
			return true;
		Q.pop();
		int emptyPos = getEmptyPos(actState);

		uint16_t neighbour = getNeighbours(emptyPos, num);
		for (int i = 0; i < num; ++i){
			//std::cout << "emptyP: "<<emptyPos << ", neighb:" << (neighbour & 15u) << std::endl;
			uint64_t nextState = getNextState(emptyPos, neighbour & 15u, actState);
			if (visitedStates.find(nextState) == visitedStates.end()) {
				Q.push(nextState);
				parent[nextState] = actState;
				level[nextState] = level[actState] + 1;
			}
			neighbour >>= 4;
		}
	}
	return false;
}

uint64_t getRandomState() {
	uint64_t randState = 0;
	std::vector<bool> v(16,0);
	while (std::count(v.begin(), v.end(), false)) {
		int r = rand() % 16;
		if (!v[r]) {
			v[r] = true;
			randState <<= 4;
			randState += unsigned(r);
		}
	}
	return randState;
}

uint64_t readFile(std::string fname) {
	std::ifstream ifs(fname);
	uint64_t input_state = 0;
	int n;
	for (int i = 0; i < 16; ++i) {
		input_state <<= 4;
		ifs >> n;
		input_state += (n + 15) % 16;
	}
	return input_state;
}
int main(int argc, char* args[]) {
	std::string filename = "puzzle.txt";
	int animation_time_sec = 30;
	
	std::map<uint64_t, uint64_t> parent;
	bool solved = false;
	int maxtime = 0;
	uint64_t state;
	uint64_t maxpuzzle;
	srand(time(0));
	//for (int i = 0; i < 50000; ++i) {
	state = readFile(filename);
	state = getRandomState();
	printState(state);
	auto t = std::clock();
	solved = solve(state, parent);
	if (std::clock() - t > maxtime) {
		maxtime = std::clock() - t;
		maxpuzzle = state;
	}
	//}
	std::cout << 1000.0 * maxtime / CLOCKS_PER_SEC << " ms" << std::endl;
	
	std::vector<uint64_t> steps;
	steps.reserve(250);
	if (!solved)
		return 0;
	std::ofstream ofs("sol.txt");
	// get the path to the start state (from the final state)
	uint64_t sol = 0x0123456789ABCDEFULL;
	steps.push_back(sol);
	while (sol != state) {
		printState(sol, ofs);
		sol = parent[sol];
		steps.push_back(sol);
	}
	
	if (SDL_Init(SDL_INIT_VIDEO) == -1)
	{
		// irjuk ki a hibat es terminaljon a program
		std::cout << "[SDL indítása]Hiba az SDL inicializálása közben: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_Window *win = nullptr;
	win = SDL_CreateWindow("Hello SDL!",				// az ablak fejléce
		100,						// az ablak bal-felsõ sarkának kezdeti X koordinátája
		100,						// az ablak bal-felsõ sarkának kezdeti Y koordinátája
		640,						// ablak szélessége
		640,						// és magassága
		SDL_WINDOW_SHOWN);			// megjelenítési tulajdonságok

									// ha nem sikerült létrehozni az ablakot, akkor írjuk ki a hibát, amit kaptunk és lépjünk ki
	if (win == nullptr)
	{
		std::cout << "[Ablak létrehozása]Hiba az SDL inicializálása közben: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_Renderer *ren = nullptr;
	ren = SDL_CreateRenderer(win, // melyik ablakhoz rendeljük hozzá a renderert
		-1,  // melyik indexú renderert inicializáljuka
			 // a -1 a harmadik paraméterben meghatározott igényeinknek megfelelõ elsõ renderelõt jelenti
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);	// az igényeink, azaz
																// hardveresen gyorsított és vsync-et beváró
	if (ren == nullptr)
	{
		std::cout << "[Renderer létrehozása]Hiba az SDL inicializálása közben: " << SDL_GetError() << std::endl;
		return 1;
	}
	SDL_Texture* tex = IMG_LoadTexture(ren, "lena.png");
	if (tex == nullptr)
	{
		std::cout << "[Kép betöltése] Hiba: " << IMG_GetError() << std::endl;
		SDL_DestroyWindow(win);
		return 1;
	}

	// véget kell-e érjen a program futása?
	bool quit = false;
	// feldolgozandó üzenet ide kerül
	SDL_Event ev;
	// egér X és Y koordinátái
	Sint32 mouseX = 0, mouseY = 0;
	int animation_time = animation_time_sec * 1000;
	int start = SDL_GetTicks();
	while (!quit)
	{
		// amíg van feldolgozandó üzenet dolgozzuk fel mindet:
		while (SDL_PollEvent(&ev))
		{
			switch (ev.type)
			{
			case SDL_QUIT:
				quit = true;
				break;
			}
		}
		int stepNum = lround(steps.size()*((double) SDL_GetTicks() - start)/animation_time);
		if (stepNum >= steps.size())
			break;
		// töröljük a hátteret fehérre
		SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
		SDL_RenderClear(ren);

		// rajzoljuk ki a betöltött képet az egékurzor köré!
		int tex_width, tex_height;
		SDL_QueryTexture(tex, nullptr, nullptr, &tex_width, &tex_height);
		uint64_t state = steps[steps.size() - stepNum - 1];
		for (unsigned i = 0; i < 16; ++i) {
			int s = getVal(state, i);
			if (s == 15)
				continue;
			SDL_Rect val_rect;
			val_rect.w = tex_width/4;
			val_rect.h = tex_height/4;
			val_rect.x = (s%4) * tex_width / 4;
			val_rect.y = (s / 4) * tex_width / 4;

			SDL_Rect pos_rect;
			pos_rect.w = 640 / 4;
			pos_rect.h = 640 / 4;
			pos_rect.x = (i % 4) * 640 / 4;
			pos_rect.y = (i / 4) * 640 / 4;

			SDL_RenderCopy(ren,				// melyik renderelõre rajzoljunk
				tex,				// melyik textúrát rajzoljuk rá
				&val_rect,			// a textúra melyik al-rect-jét
				&pos_rect);		// a renderelõ felületének mely részére
		}

		SDL_RenderPresent(ren);
	}
	std::this_thread::sleep_for(std::chrono::seconds(1));
	SDL_DestroyTexture(tex);
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);

	SDL_Quit();

	return 0;
}