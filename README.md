# Time Series Pattern Recognition with OpenMP

Analisi prestazionale e implementazione di un algoritmo di **Pattern Recognition** su serie temporali basato sulla metrica **SAD (Sum of Absolute Differences)**. Il progetto confronta l'efficienza di un'esecuzione sequenziale rispetto a diverse strategie di calcolo parallelo in **C++** utilizzando **OpenMP**.

## 📌 Descrizione del Progetto
L'obiettivo è identificare la posizione di miglior match di una sequenza pattern (Query) all'interno di un dataset di serie temporali. La somiglianza viene calcolata tramite la formula SAD:

$$SAD = \sum_{i=0}^{n-1} |S[i] - Q[i]|$$

Dove $S$ è la finestra della serie temporale e $Q$ è la query pattern.

## 🛠️ Strategie Implementate
Il software analizza tre diversi approcci alla parallelizzazione:
* **V1 - Bottleneck**: Versione con sezione critica all'interno del loop principale. Dimostra l'impatto della contesa dei thread (lock overhead).
* **V2 - Standard**: Ottimizzazione tramite variabili locali per thread, riducendo la sincronizzazione al minimo indispensabile.
* **V3 - User Defined Reduction**: Implementazione avanzata tramite clausola `reduction` personalizzata per oggetti C++, garantendo codice pulito e massime performance.

## 📊 Performance & Risultati
I test sono stati condotti su architettura **Intel Core i7-9750H** (6 Core / 12 Thread).

| Scenario | Dataset (Punti) | Speedup Max (12 Thread) | Efficienza |
| :--- | :---: | :---: | :---: |
| **Low Load** | 5M | **9.36x** | Ottima |
| **Med Load** | 10M | **8.84x** | Ottima |
| **High Load** | 50M | **8.91x** | Massima |



### Analisi dell'Hyper-Threading
Il raggiungimento di uno speedup di **~9x** su **6 core fisici** evidenzia un utilizzo ottimale dell'**Hyper-Threading**. Essendo l'algoritmo *compute-bound*, l'uso di 12 thread logici permette di saturare le pipeline di calcolo della CPU, abbattendo drasticamente i tempi di esecuzione (da 24s a 2.7s nel caso High Load).

## 🚀 Getting Started

### Requisiti
* Compilatore C++17 (MSVC, GCC o Clang)
* Supporto per **OpenMP 4.5+**
* **CMake** 3.20 o superiore

### Build & Run
1.  Clona il repo: `git clone https://github.com/tuo-username/nome-repo.git`
2.  Crea la cartella build: `mkdir build && cd build`
3.  Genera i file: `cmake ..`
4.  Compila: `cmake --build . --config Release`
5.  Esegui: `./ParallelComputing_PatternRecognition` (assicurati di avere i dati in `../data`)

## 📂 Struttura del Repository
* `src/`: File sorgenti (`.cpp`, `.h`).
* `data/`: Cartella per i dataset CSV (non inclusi se troppo grandi).
* `Results/`: Log dei benchmark e report dei tempi.

---
**Autore:** Alessia Donati  
*Progetto per il corso di Parallel Computing - Università degli Studi di Firenze.*
