#include "Lab1.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <windows.h>

using namespace std;

//Константы, которые определяют внутреннюю экономику
constexpr float man_eat = 20.0f;//количество еды, которое съедает один человек в год
constexpr int man_do_ground_MIN = 1;//минимальное количество участков, которое может обработать один человек в год
constexpr int man_do_ground_MAX = 10;//максимальное количество участков, которое может обработать один человек в год

constexpr float ground_take_food = 0.5f;//количество еды, которое берет один участок в год для возделывания
constexpr float ground_give_food_MIN = 1.0f;//минимальное количество еды, которое дает один возделанный участок в год для потребления
constexpr float ground_give_food_MAX = 6.0f;//максимальное количество еды, которое дает один возделанный участок в год для потребления

constexpr float rats_eat_multiplier_min = 0.0f;//минимальный множитель, на который умножается количество еды, которое съедают крысы в год
constexpr float rats_eat_multiplier_max = 0.07f;//максимальный множитель, на который умножается количество еды, которое съедают крысы в год

constexpr float people_death_threshold = 0.45f;//порог смертей населения за раунд (от общего кол-ва народа), после которого игрок проигрывает

constexpr float plague_probability = 0.15f;//веорятность чумы

//Константы для настройки игры
constexpr int max_years = 10;


//Основные параметры игры (количество людей, еды и участков)
int count_people_alive = 100;
int count_people_dead_from_hunger = 0;
int total_deaths_from_hunger = 0;
float count_food = 2800;
int count_grounds = 1000;
int count_curr_year = 1;

//Случайные переменные, определяемые каждый раунд
float ground_price; //Цена на участок
int newcomers = 0;
bool was_plague = false;
int wheat_harvested = 0;
float wheat_per_acre = 0;
int wheat_eaten_by_rats = 0;

//Новоприбывшие
int get_newcomers(int death_count, float acre_coll_count, float curr_food_count)
{
	int result = (int)(death_count/2 + (5-acre_coll_count) * curr_food_count/600 + 1);
	if(result < 0) result = 0;
	else if(result > 50) result = 50;
	return result;
}

void save_game() {
	ofstream file("hammurabi_save.txt");
	if (file.is_open()) {
		file << count_people_alive << endl;
		file << count_people_dead_from_hunger << endl;
		file << total_deaths_from_hunger << endl;
		file << count_food << endl;
		file << count_grounds << endl;
		file << count_curr_year << endl;
		file << ground_price << endl;
		file << newcomers << endl;
		file << was_plague << endl;
		file << wheat_harvested << endl;
		file << wheat_per_acre << endl;
		file << wheat_eaten_by_rats << endl;
		file.close();
		cout << "Игра сохранена!\n";
	}
}

bool load_game() {
	ifstream file("hammurabi_save.txt");
	if (file.is_open()) {
		file >> count_people_alive;
		file >> count_people_dead_from_hunger;
		file >> total_deaths_from_hunger;
		file >> count_food;
		file >> count_grounds;
		file >> count_curr_year;
		file >> ground_price;
		file >> newcomers;
		file >> was_plague;
		file >> wheat_harvested;
		file >> wheat_per_acre;
		file >> wheat_eaten_by_rats;
		file.close();
		return true;
	}
	return false;
}

void show_statistics_per_round(){
	cout << "\n\nМой повелитель, соизволь поведать тебе\n";
	cout << "в году " << count_curr_year-1 << " твоего правления:\n";
	
	if (count_curr_year > 1) {
		if (count_people_dead_from_hunger > 0) {
			cout << count_people_dead_from_hunger << " человек умерло от голода\n";
		}
		
		if (newcomers > 0) {
			cout << newcomers << " человек прибыло в город\n";
		}
		
		if (was_plague) {
			cout << "Чума уничтожила половину населения!\n";
		}
		
		cout << "Мы собрали " << wheat_harvested << " бушелей пшеницы, по " << wheat_per_acre << " бушеля с акра\n";
		cout << "Крысы уничтожили " << wheat_eaten_by_rats << " бушелей пшеницы\n";
	}
	
	cout << "Население города сейчас составляет " << count_people_alive << " человек\n";
	cout << "Город сейчас занимает " << count_grounds << " акров\n";
	cout << "1 акр земли сейчас стоит " << (int)ground_price << " бушелей\n";
	cout << "В амбарах сейчас " << (int)count_food << " бушелей пшеницы\n";
	cout << "(Для выхода из игры введите 'q' в любом поле ввода)\n\n";
	
	// Сбрасываем статистику предыдущего раунда
	count_people_dead_from_hunger = 0;
	newcomers = 0;
	was_plague = false;
}

bool get_player_input_at_this_year()
{
	int acres_to_buy = 0, acres_to_sell = 0, food_to_eat = 0, ground_to_seed = 0;
	string input;

	cout << "Сколько акров земли повелеваешь купить? ";
	cin >> input;
	if (input == "q" || input == "Q") {
		save_game();
		exit(0);
	}
	acres_to_buy = atoi(input.c_str());

	if (acres_to_buy <= 0)
	{
		cout << "Сколько акров земли повелеваешь продать? ";
		cin >> input;
		if (input == "q" || input == "Q") {
			save_game();
			exit(0);
		}
		acres_to_sell = atoi(input.c_str());
	}

	cout << "Сколько бушелей пшеницы повелеваешь съесть? ";
	cin >> input;
	if (input == "q" || input == "Q") {
		save_game();
		exit(0);
	}
	food_to_eat = atoi(input.c_str());

	cout << "Сколько акров земли повелеваешь засеять? ";
	cin >> input;
	if (input == "q" || input == "Q") {
		save_game();
		exit(0);
	}
	ground_to_seed = atoi(input.c_str());

	//Проверки
	if (acres_to_buy < 0 || ground_price*acres_to_buy > count_food) {
		cout << "О, повелитель, пощади нас! У нас только " << (int)count_food << " бушелей пшеницы!\n";
		return get_player_input_at_this_year();
	}
	if (acres_to_sell < 0 || acres_to_sell > count_grounds)
	{
		cout << "О, повелитель, пощади нас! У нас только " << count_grounds << " акров земли!\n";
		return get_player_input_at_this_year();
	}
	if (food_to_eat < 0 || food_to_eat > count_food)
	{
		cout << "О, повелитель, пощади нас! У нас только " << (int)count_food << " бушелей пшеницы!\n";
		return get_player_input_at_this_year();
	}
	if (ground_to_seed < 0 || ground_to_seed > count_grounds)
	{
		cout << "О, повелитель, пощади нас! У нас только " << count_grounds << " акров земли!\n";
		return get_player_input_at_this_year();
	}
	if (ground_to_seed * ground_take_food > count_food)
	{
		cout << "О, повелитель, пощади нас! У нас только " << (int)count_food << " бушелей пшеницы на семена!\n";
		return get_player_input_at_this_year();
	}
	if (ground_to_seed > count_people_alive * man_do_ground_MAX)
	{
		cout << "О, повелитель, пощади нас! У нас только " << count_people_alive << " человек!\n";
		return get_player_input_at_this_year();
	}

	// Сохраняем начальное население для проверки
	int people_at_start = count_people_alive;
	
	// Применяем действия игрока
	count_food -= acres_to_buy * ground_price;
	count_grounds += acres_to_buy;
	
	count_food += acres_to_sell * ground_price;
	count_grounds -= acres_to_sell;
	
	count_food -= ground_to_seed * ground_take_food;
	
	// Сбор урожая
	wheat_per_acre = ground_give_food_MIN + (float)(rand() % (int)(ground_give_food_MAX - ground_give_food_MIN + 1));
	wheat_harvested = (int)(ground_to_seed * wheat_per_acre);
	count_food += wheat_harvested;
	
	// Крысы
	float rats_multiplier = rats_eat_multiplier_min + (float)rand() / RAND_MAX * (rats_eat_multiplier_max - rats_eat_multiplier_min);
	wheat_eaten_by_rats = (int)(count_food * rats_multiplier);
	count_food -= wheat_eaten_by_rats;
	
	// Кормление населения
	int people_fed = (int)(food_to_eat / man_eat);
	if (people_fed > count_people_alive) people_fed = count_people_alive;
	
	count_people_dead_from_hunger = count_people_alive - people_fed;
	count_people_alive = people_fed;
	total_deaths_from_hunger += count_people_dead_from_hunger;
	
	// Проверка на проигрыш
	float death_rate = (float)count_people_dead_from_hunger / people_at_start;
	if (death_rate > people_death_threshold) {
		return false;
	}
	
	// Новоприбывшие
	newcomers = get_newcomers(count_people_dead_from_hunger, wheat_per_acre, count_food);
	count_people_alive += newcomers;
	
	// Чума
	if ((float)rand() / RAND_MAX < plague_probability) {
		was_plague = true;
		count_people_alive = count_people_alive / 2;
	}
	
	return true;
}

void show_final_statistics() 
{
	float P = (float)total_deaths_from_hunger / count_curr_year;
	float L = (float)count_grounds / (count_people_alive > 0 ? count_people_alive : 1);
	
	cout << "\n\n========================================\n";
	cout << "ИТОГИ ВАШЕГО ПРАВЛЕНИЯ\n";
	cout << "========================================\n";
	cout << "Вы правили " << count_curr_year << " лет\n";
	cout << "Средний процент умерших от голода за год: " << P << "%\n";
	cout << "Акров земли на одного жителя: " << L << "\n\n";
	
	if (P > 33 || L < 7) {
		cout << "Из-за вашей некомпетентности в управлении, народ устроил бунт,\n";
		cout << "и изгнал вас из города. Теперь вы вынуждены влачить жалкое\n";
		cout << "существование в изгнании.\n";
	}
	else if (P > 10 || L < 9) {
		cout << "Вы правили железной рукой, подобно Нерону и Ивану Грозному.\n";
		cout << "Народ вздохнул с облегчением, и никто больше не желает видеть\n";
		cout << "вас правителем.\n";
	}
	else if (P > 3 || L < 10) {
		cout << "Вы справились вполне неплохо, у вас, конечно, есть недоброжелатели,\n";
		cout << "но многие хотели бы увидеть вас во главе города снова.\n";
	}
	else {
		cout << "Фантастика! Карл Великий, Дизраэли и Джефферсон вместе\n";
		cout << "не справились бы лучше!\n";
	}
}

int main()
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	srand(time(NULL));
	
	cout << "===========================================\n";
	cout << "   ХАММУРАПИ - ПРАВИТЕЛЬ ВАВИЛОНА\n";
	cout << "===========================================\n";
	
	// Проверка на сохраненную игру
	ifstream check_file("hammurabi_save.txt");
	if (check_file.good()) {
		check_file.close();
		cout << "Обнаружена сохраненная игра!\n";
		cout << "1 - Продолжить сохраненную игру\n";
		cout << "2 - Начать новую игру\n";
		cout << "Ваш выбор: ";
		int choice;
		cin >> choice;
		
		if (choice == 1) {
			if (load_game()) {
				cout << "Игра загружена!\n";
			}
		} else {
			// Удаляем старое сохранение
			remove("hammurabi_save.txt");
		}
	}
	
	cout << "\nУправляйте городом в течение 10 лет.\n";
	cout << "Заботьтесь о людях, земле и пшенице!\n";

	for (; count_curr_year <= max_years; count_curr_year++) 
	{
		// Генерируем цену земли
		ground_price = 17.0f + (float)(rand() % 10);
		
		// Показываем статистику
		show_statistics_per_round();
		
		// Проверка на конец игры
		if (count_people_alive <= 0) {
			cout << "\n\nВсе жители умерли! Игра окончена.\n";
			show_final_statistics();
			remove("hammurabi_save.txt");
			return 0;
		}
		
		// Получаем действия игрока
		bool success = get_player_input_at_this_year();
		
		if (!success) {
			cout << "\n\nБолее 45% населения умерло от голода за раунд!\n";
			cout << "Народ восстал и сверг вас с престола!\n";
			show_final_statistics();
			remove("hammurabi_save.txt");
			return 0;
		}
	}
	
	// Конец игры после 10 лет
	show_final_statistics();
	remove("hammurabi_save.txt");
	return 0;
}