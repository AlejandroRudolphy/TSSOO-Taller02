#include <global.h>
#include <checkArgs.hpp>

uint64_t* arreglo = nullptr;
uint64_t sumaArreglo = 0;

std::vector<std::thread *> threads;
std::vector<std::thread *> threadSuma;
std::mutex mux;


//En esta sección se utilizo una de la funcion thread-safe del ejemplo, para generar números randómicos y llenar los arreglos

void fillArray(size_t beginIdx, size_t endIdx, size_t limInferior, size_t limSuperior){
	std::random_device device;
	std::mt19937 rng(device());
	std::uniform_int_distribution<> unif(limInferior,limSuperior);

	for(size_t i = beginIdx; i < endIdx; i++){
		arreglo[i] = unif(rng);
	}
}


void sumaParcial(uint64_t &sumaArreglo, uint32_t beginIdx, uint32_t endIdx){
	sumaArreglo=0;
	mux.lock();
	for(uint32_t i = beginIdx; i < endIdx; i++){
		sumaArreglo += arreglo[i];
	}
	mux.unlock();
}


int main(int argc, char** argv){

	uint64_t totalElementos;
	uint32_t numThreads;
	uint32_t limInf;
	uint32_t limSup;

	auto argumentos = (std::shared_ptr<checkArgs>) new checkArgs(argc, argv);

	totalElementos = argumentos->getArgs().tamProblema;
	numThreads = argumentos->getArgs().numThreads;
	limInf = argumentos->getArgs().limInferior;
	limSup = argumentos->getArgs().limSuperior;

	std::cout << "Elementos: " << totalElementos << std::endl;
	std::cout << "Threads: " << numThreads << std::endl;
	std::cout << "Límite inferior: " << limInf << std::endl;
	std::cout << "Límite superior: " << limSup << std::endl;


	//Etapa de llenado

	//Secuencial
	arreglo = new uint64_t[totalElementos];

	auto start = std::chrono::high_resolution_clock::now();

	fillArray(0, totalElementos, limInf, limSup);

	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	auto tiempoLlenadoTotal_S = elapsed.count();

	delete[] arreglo;
	//Paralelo
	arreglo = new uint64_t[totalElementos];

	start = std::chrono::high_resolution_clock::now();

	//En esta sección se crean los threads en base al valor numThreads ingresado y se distribuye el trabajo entre ellos
	for(size_t i=0; i < numThreads; i++){
		threads.push_back(new std::thread(fillArray, i*(totalElementos)/numThreads, (i+1)*(totalElementos)/numThreads, limInf, limSup));
	}

	for(auto &thFilled : threads){
		thFilled->join();
	}

	end = std::chrono::high_resolution_clock::now();
	elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
	auto tiempoLlenadoTotal_P = elapsed.count();

	//Etapa de suma

	//Secuencial
	uint64_t sumaSecuencial=0;

	start = std::chrono::high_resolution_clock::now();

	sumaParcial(std::ref(sumaSecuencial), 0, totalElementos);

	end = std::chrono::high_resolution_clock::now();
	elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	auto tiempoTotalSuma_S = elapsed.count();

	//Paralela
	uint64_t sumaParalela=0;
	start = std::chrono::high_resolution_clock::now();
	//En esta sección se crean los threads en base al valor numThreads ingresado y se distrubuye el trabajo entre ellos
	for(size_t i=0;i<numThreads;i++){
		threadSuma.push_back(new std::thread(sumaParcial, std::ref(sumaParalela), i*(totalElementos)/numThreads, (i+1)*(totalElementos)/numThreads));
	}
	for(auto &thSuma : threadSuma){
		thSuma->join();
	}
	end = std::chrono::high_resolution_clock::now();
	elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	auto tiempoTotalSuma_P = elapsed.count();

	std::cout << "Suma secuencial total: " << sumaSecuencial << std::endl;
	std::cout << "Suma en paralelo total: " << sumaParalela << "\n" << std::endl;
	std::cout << "Tiempos de ejecución etapa de llenado: \n" << std::endl;
	std::cout << "Tiempo Llenado Secuencial: " << tiempoLlenadoTotal_S << "[ms]" << std::endl;
	std::cout << "Tiempo Llenado Paralelo: " << tiempoLlenadoTotal_P << "[ms]\n" << std::endl;
	std::cout << "Tiempos de ejecución etapa de suma: \n" << std::endl;
	std::cout << "Tiempo Suma Secuencial: " << tiempoTotalSuma_S << "[ms]" << std::endl;
	std::cout << "Tiempo Suma Paralela: " << tiempoTotalSuma_P << "[ms]\n" << std::endl;

	return(EXIT_SUCCESS);
}

