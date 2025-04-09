# Lab 1. Implementation of SkipList 보고서

# 1. 서론

### 1.1. 과제 개요 및 목적

| 학과 | 소프트웨어학과 |
| --- | --- |
| 학번 | 32213538 |
| 이름 | 이지수 |
| 수업명 | 오픈소스SW분석(빅데이터) 2분반 |
| 과제명 | Implementation of SkipList  |
| 상세내용 | - SkipList 자료구조를 직접 구현하여, 구조적 원리와 효율성 이해
- 실제 데이터 기반 성능 실험을 통한 분석 및 고찰 |

### 1.2. 제공된 파일 목록 및 역할

| 파일명 | 설명 |
| --- | --- |
| skiplist.h | SkipList의 기능 구현 파일 (학생 구현 대상) |
| skiplist_test.cc | Benchmark 기능 포함된 테스트 코드 |
| zipf.h / zipf.cc | Zipfian 분포 생성기 |
| latest-generator.h / latest-generator.cc | 최신 분포 (latest) 생성기 |
| Makefile | 빌드 자동화 파일 |

### 1.3. 구현 범위와 수정한 부분

- 필수 구현 함수: Insert, Delete, Contains, Scan, RandomLevel
- 추가 구현 및 수정
    - 중복 Key 처리 로직 개선
    - 메모리 누수 방지 로직 삽입
    - Scan, Insert의 효율적인 구현 전략 비교 실험
    - 정확도 테스트를 위한 별도 테스트 코드 작성

---

# 2. 기능별 구현

### 2.1.1. RandomLevel

```cpp
template<typename Key>
int SkipList<Key>::RandomLevel() {
    int level = 1; // 기본 level 1부터 시작
    while (((float)rand() / RAND_MAX) < probability && level < max_level) { 
    // 확률 적으로 level 증가
        level++;
    }
    return level; // 최종 level 반환
}
```

### 2.1.2. Insert

```cpp
template<typename Key>
void SkipList<Key>::Insert(const Key& key) {
    int node_level = RandomLevel(); // 새로운 노드에 할당할 레벨을 확률적으로 결정
    Node* node = new Node(key, node_level); // 해당 레벨을 가진 노드 생성
    Node* p = head; // 포인터 초기화
    // 각 레벨에서 삽입 위치 직전 노드(p)를 저장
    std::vector<Node*> update(max_level, nullptr); 

    // 각 level에서 삽입할 key의 위치를 찾음
    for (int level = node_level - 1; level >= 0; level--) {
        // 현재 level에서 key보다 작은 노드일 경우 next로 이동
        while (p->next[level] != nullptr && compare_(key, p->next[level]->key) > 0) {
            p = p->next[level];
        }
        update[level] = p; // 삽입 위치 바로 전 노드를 저장
    }

    // 중복 key가 이미 존재할 경우 삽입하지 않음
    if (update[0]->next[0] != nullptr && compare_(key, update[0]->next[0]->key) == 0) {
        delete node; // 할당한 노드는 삭제하여 메모리 누수 방지
        return;
    }

    // 각 레벨에 대해 새 노드를 삽입
    for (int i = 0 ; i < node_level ; i++) {
        node->next[i] = update[i]->next[i]; // 새 노드가 가리킬 대상 설정
        update[i]->next[i] = node; // 이전 노드가 새 노드를 가리키도록 설정
    }
}
```

### 2.1.3. Contains

```cpp
template<typename Key>
bool SkipList<Key>::Contains(const Key& key) const {
    Node* p = head; // 탐색 시작 지점

    // 위에서부터 내려오며 탐색 -> max_level - 1부터 시작
    for (int level = max_level - 1; level >= 0; level--) {
        // 현재 level에서 key보다 작은 노드일 경우 next로 이동
        while (p->next[level] != nullptr && compare_(key, p->next[level]->key) > 0) {
            p = p->next[level];
        }
        // 현재 레벨에서 key와 같은 값을 가진 노드를 찾으면 true 반환
        if (p->next[level] != nullptr && compare_(key, p->next[level]->key) == 0) {
            return true;
        }
        // key보다 큰 값을 만났거나 끝까지 간 경우 → 아래 레벨로 내려가 탐색 계속
    }

    // 모든 레벨에서 key를 찾지 못한 경우 false 반환
    return false;
}
```

### 2.1.4. Delete

```cpp
template<typename Key>
bool SkipList<Key>::Delete(const Key& key) const {
    Node* p = head;       // 탐색 시작 지점
    Node* d = nullptr;    // 실제로 삭제할 노드를 저장할 포인터

    // 가장 높은 레벨부터 아래로 내려가며 탐색
    for (int level = max_level - 1; level >= 0; level--) {
        while (p->next[level] != nullptr) {
            int cmp = compare_(key, p->next[level]->key); // key와 다음 노드의 key 비교

						// 삭제할 key를 찾은 경우 (key와 동일한 키를 가진 노드를 찾은 경우)
            if (cmp == 0) { 
                d = p->next[level];               // 삭제 대상 노드 저장
                p->next[level] = d->next[level];  // 삭제 대상 노드와의 연결 해제
            } else if (cmp > 0) { 
                p = p->next[level]; // 찾는 key보다 작으면 계속 다음 노드로 이동
            } else { // 찾는 key보다 크면 현재 레벨에서는 찾는 key가 없음 → 다음 레벨(아래 레벨)로 이동
                break;
            }
        }
    }

    if (d != nullptr) { // 삭제 대상 노드를 찾았다면
        delete d;        // 한 번만 delete 수행 
        return true;     // 삭제 성공
    }

    return false; // 삭제 대상 key가 없을 경우 false
}

```

### 2.1.5. Scan

```cpp
template<typename Key>
std::vector<Key> SkipList<Key>::Scan(const Key& key, const int scan_num) {
    // key 이상(>= key)인 첫 노드부터 시작하여 scan_num개의 key를 수집
    Node* p = head;               // 탐색 시작 지점
    int cnt = scan_num;           // 수집할 key의 개수
    std::vector<Key> result = {}; // 리턴할 결과 벡터

		// 가장 높은 레벨부터 아래로 내려가며 탐색
    for (int level = max_level - 1; level >= 0; level--) {
        // 현재 레벨에서 key보다 작은 노드들을 건너뜀
        while (p->next[level] != nullptr && compare_(key, p->next[level]->key) > 0) {
            p = p->next[level];
        }
    }

    // Level 0에서 key 이상인 노드부터 시작하여 scan_num개 수집
    while (cnt > 0 && p->next[0] != nullptr) {
        result.push_back(p->next[0]->key); // 다음 노드의 key를 result에 추가 
				p = p->next[0];                    // 다음 노드로 이동
        cnt--;                             // 남은 수집 개수 감소
    }
    return result; // 수집된 결과 반환
}
```

---

# 3. Insert 구현 과정

## **3.1. First Thinking**

- 각 레벨마다 삽입할 key의 위치를 찾아, 노드 사이에 삽입하는 방식으로 구현을 구상함
- head부터 시작하여, 현재 레벨의 노드들을 순차적으로 따라가며 key보다 큰 값을 가진 노드를 만날 때까지 탐색
- 삽입할 노드의 level부터 0까지 한 단계씩 내려가며 탐색 및 삽입을 반복함
    
    → 각 레벨에서 탐색은 head->next[level]부터 시작함
    
    → 탐색 중 p는 삽입 위치 바로 앞 노드를 가리키도록 설정함
    
    → while문을 통해 p->next[level]의 key가 삽입 대상보다 크거나 같을 때까지 이동
    
    → 탐색이 끝난 후 p와 p->next[level] 사이에 새로운 노드를 삽입
    
    → 이후 한 단계 아래의 레벨로 내려가 동일한 작업을 반복 수행함
    
- 초기 구현 코드

```cpp
template<typename Key>
void SkipList<Key>::Insert(const Key& key) {
    int level = RandomLevel();
    Node* node = new Node(key, level);

    for (int i = 0 ; i < level; i++) {
        Node* p = head; 
        while (p->next[i] != nullptr && compare_(key, p->next[i]->key) > 0) {
            p = p->next[i];
        }
        node->next[i] = p->next[i];
        p->next[i] = node;
    }
}
```

## 3.2. Error 1 - 중복된 Key 삽입 문제

### (1) 현상

- 동일한 key 값을 가진 노드가 중복 삽입되는 문제 발생
    
    ![](https://velog.velcdn.com/images/windylung/post/aa030475-c626-4075-9cc5-9d80f2d10823/image.png)
    

### (2) 원인

- while문의 조건을 `compare_(key, p->next[i]->key) > 0`으로 설정함
- key보다 크지 않으면 무조건 삽입이 이루어져, 중복 key가 삽입됨

### **(3) 해결 방안**

- key가 이미 존재할 경우 삽입을 중단하도록 조건 추가

```cpp
    while (level >= 0) {
        while (p->next[level] != nullptr && compare_(key, p->next[level]->key) >= 0) {
						// key 값보다 작을 경우 다음으로 이동
            p = p->next[level];
        }
        if (p->next[level] != nullptr && compare_(key, p->next[level]->key) == 0) {
	        // key 값과 같을 경우 return
	        return ;
	       } 
        node->next[level] = p->next[level]; // p와 p->next[level] 사이에 node 삽입
        p->next[level] = node;
        level -= 1; // key보다 key값이 작은 p -> p의 한 단계 아래에서 insert 반복
    }
```

## 3.3. Error 2 - 비효율적인 삽입 로직

### (1) 원인

- 각 level마다 `p = head`로 초기화하여 처음부터 다시 탐색함
- 상위 레벨에서 얻은 정보를 활용하지 못함 → SkipList의 장점 무시
    
    ![[그림] 
    Key = 5일 때, level = n에서 4와 7 사이에 노드 삽입 수행.
    level = n -1 에서 2부터 다시 탐색이 진행 됨. ](Lab%201%20Implementation%20of%20SkipList%20%E1%84%87%E1%85%A9%E1%84%80%E1%85%A9%E1%84%89%E1%85%A5%201cf63aebf247808bb92ed8d7bfe8c949/image.png)
    
    [그림] 
    Key = 5일 때, level = n에서 4와 7 사이에 노드 삽입 수행.
    level = n -1 에서 2부터 다시 탐색이 진행 됨. 
    

### (2) 해결 방안

- 상위 레벨에서 탐색한 위치를 활용하여, 하위 레벨에서 이어서 탐색 수행
    
    ![[그림] p→next[level-1]부터 탐색 시작](Lab%201%20Implementation%20of%20SkipList%20%E1%84%87%E1%85%A9%E1%84%80%E1%85%A9%E1%84%89%E1%85%A5%201cf63aebf247808bb92ed8d7bfe8c949/image%201.png)
    
    [그림] p→next[level-1]부터 탐색 시작
    

```cpp
    while (level >= 0) {
				// key 값보다 작을 경우 다음으로 이동
        while (p->next[level] != nullptr && compare_(key, p->next[level]->key) >= 0) { 
            p = p->next[level];
        }
        // key 값과 같을 경우 return
        if (p->next[level] != nullptr && compare_(key, p->next[level]->key) == 0) {return ;} 
        node->next[level] = p->next[level]; // p와 p->next[level] 사이에 node 삽입
        p->next[level] = node;
        level -= 1; // key보다 key값이 작은 p -> p의 한 단계 아래에서 insert 반복
    }
```

## 3.4. Error 3 - level과 index 혼용 문제

### (1) 원인

- RandomLevel()의 반환값(level)을 그대로 next 배열의 index로 사용함
- level은 1부터 시작하지만, next 배열의 index는 0부터 시작함

### (2) 해결 방안

- level에서 1을 감소시켜 index로 사용하거나
- Node 생성 시 next 배열 크기를 level+1로 설정하여 0부터 시작하도록 변경함

```cpp
SkipList<Key>::Node::Node(Key key, int level)
    : key(key), next(level + 1, nullptr) {}
```

- 이 경우 `next[0]`이 항상 비게 되는 문제가 발생함.
- 따라서 기존 방식 유지 + index = level - 1 로 일관되게 처리함

## 3.5. Error 4 - 중복 Key 검출 시 메모리 누수

### (1) 현상

- 중복 key 삽입 도중 return을 하더라도, 이미 생성된 node는 delete 되지 않음
    
    ![[그림]
    삽입 과정에서 중복 키가 발견되면 이미 일부 레벨에 노드가 삽입된 상태일 수 있음 ](Lab%201%20Implementation%20of%20SkipList%20%E1%84%87%E1%85%A9%E1%84%80%E1%85%A9%E1%84%89%E1%85%A5%201cf63aebf247808bb92ed8d7bfe8c949/image%202.png)
    
    [그림]
    삽입 과정에서 중복 키가 발견되면 이미 일부 레벨에 노드가 삽입된 상태일 수 있음 
    

### (2) 해결 방안

- 사전에 `Contains`를 통해 중복 여부를 확인하고 삽입을 중단함

```cpp
void SkipList<Key>::Insert(const Key& key) {
    if (Contains(key)) {
        return;
    }
 //...
```

- 단점 : Insert 수행 전 Contains 수행하기에 → SkipList 2회 순회

## 3.6. 최종 개선 로직 - 중복 검출 및 삽입 최적화

- 삽입 위치를 저장할 update 배열 선언
- 탐색 중 중복 key 발견 시 삽입 중단 및 node 해제
- 삽입은 탐색 종료 후 한 번에 수행

```cpp
template<typename Key>
void SkipList<Key>::Insert(const Key& key) {
    int node_level = RandomLevel();
    Node* node = new Node(key, node_level);
    Node* p = head;
    std::vector<Node*> update(node_level, nullptr);

    for (int level = node_level - 1; level >= 0; level--) {
        while (p->next[level] != nullptr && compare_(key, p->next[level]->key) >= 0) {
            p = p->next[level];
        }
        if (compare_(key, p->next[level]->key) == 0) {
            delete node;
            return;
        }
        update[level] = p;
    }

    for (int level = 0; level < node_level; level++) {
        node->next[level] = update[level]->next[level];
        update[level]->next[level] = node;
    }
}

```

## 3.7. 실험 1 - 중복 Key 검사 전략 비교

### 3.7.1. 실험 목적

- 기존 방식은 모든 레벨에서 중복 키를 검사함
- 레벨이 높아질수록 노드 수가 적기 때문에, 중복 검사 비용이 낭비될 수 있음
- 따라서 Level 0에서만 중복 검사를 수행하는 방식과 비교가 필요함

| 전략 | 설명 |
| --- | --- |
| A | 모든 레벨에서 중복 key 검사 |
| B | Level 0에서만 중복 key 검사 |

### 3.7.2. 구현 방법

```cpp
// 전략 A - 모든 레벨에서 중복 Key 검사
template<typename Key>
void SkipList<Key>::Insert(const Key& key) {
    int node_level = RandomLevel(); // 새 노드에 할당할 레벨 결정
    Node* node = new Node(key, node_level); // 새 노드 생성
    Node* p = head; // head부터 탐색 시작
    std::vector<Node*> update(max_level, nullptr); // 각 레벨별 삽입 위치 직전 노드 저장

    // 모든 레벨에서 중복 키 여부 확인
    for (int level = node_level - 1; level >= 0; level--) {
        // 현재 레벨에서 key보다 작은 노드를 찾아 이동
        while (p->next[level] != nullptr && compare_(key, p->next[level]->key) > 0) {
            p = p->next[level];
        }

        // 중복 key 발견 시 노드 삭제 후 삽입 중단
        if (p->next[level] != nullptr && compare_(key, p->next[level]->key) == 0) {
            delete node;
            return;
        }

        update[level] = p; // 해당 레벨의 삽입 위치 전 노드 저장
    }

    // 삽입 위치에 새 노드를 연결
    for (int i = 0; i < node_level; i++) {
        node->next[i] = update[i]->next[i];
        update[i]->next[i] = node;
    }
}
```

```cpp
// 전략 B - Level 0에서만 중복 검사
template<typename Key>
void SkipList<Key>::Insert(const Key& key) {
    int node_level = RandomLevel(); // 새 노드에 할당할 레벨 결정
    Node* node = new Node(key, node_level); // 새 노드 생성
    Node* p = head; // head부터 탐색 시작
    std::vector<Node*> update(max_level, nullptr); // 각 레벨별 삽입 위치 직전 노드 저장

    // 삽입 위치 찾기 (중복 확인은 하지 않음)
    for (int level = node_level - 1; level >= 0; level--) {
        while (p->next[level] != nullptr && compare_(key, p->next[level]->key) > 0) {
            p = p->next[level];
        }
        update[level] = p;
    }

    // 오직 Level 0에서만 중복 키 여부 확인
    if (update[0]->next[0] != nullptr && compare_(key, update[0]->next[0]->key) == 0) {
        delete node;
        return;
    }

    // 삽입 위치에 새 노드를 연결
    for (int i = 0; i < node_level; i++) {
        node->next[i] = update[i]->next[i];
        update[i]->next[i] = node;
    }
}

```

### 3.7.3. 실험 환경

```cpp
#include <chrono>
#include <iostream>
#include <random>
#include "skiplist.h"

typedef uint64_t Key;
typedef std::chrono::high_resolution_clock Clock;

void CompareInsertDuplicateStrategies(int data_size, bool use_strategy_a) {
    const int repeat = 100;
    double total_time = 0;

    for (int i = 0; i < repeat; ++i) {
        SkipList<Key> sl;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<Key> dist(1, data_size);

        auto start = Clock::now();

        for (int j = 0; j < data_size; ++j) {
            Key k = dist(gen);

            if (use_strategy_a) {
                // 전략 A 코드 사용 (직접 삽입)
                }
            } else {
								// 전략 B 코드 사용 (직접 삽입)
            }

        skip_insert:
            continue;
        }

        auto end = Clock::now();
        total_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    }

    double avg_time = total_time / repeat / 1000.0;
    std::cout << (use_strategy_a ? "전략 A (모든 레벨 중복 검사)" : "전략 B (Level 0에서만 검사)");
    std::cout << " 평균 시간: " << avg_time << " µs" << std::endl;
}

int main() {
    int data_size = 10000;
    CompareInsertDuplicateStrategies(data_size, true);  // 전략 A
    CompareInsertDuplicateStrategies(data_size, false); // 전략 B
    return 0;
}

```

| 항목 | 내용 |
| --- | --- |
| 데이터 개수 | 10,000 |
| 반복 횟수 | 100회 |
| 기기 | MacBook Air (Apple Silicon) |
| 컴파일러 | g++ -std=c++17 |
| 측정 단위 | 평균 삽입 시간 (마이크로초) |

### 3.7.4. 실험 결과

| 실행 | 전략 A (모든 레벨) | 전략 B (Level 0만) | 차이 |
| --- | --- | --- | --- |
| 1회 | 120,889 µs | 124,793 µs | -3,904 µs |
| 2회 | 122,629 µs | 121,918 µs | 711 µs |
| 3회 | 125327 µs | 124189 µs | 1,138 µs |
- 두 전략 모두 성능 차이는 크지 않으며 오히려 실행마다 차이가 상반되기도 함을 알 수 있음

## 3.8. 실험 2 - 탐색 시작 레벨 전략 비교

### 3.8.1 실험 목적

- 기존에는 Node가 삽입되는 node_level부터 level 1까지 탐색하는 방식 사용
- SkipList의 장점을 극대화하기 위해, max_level부터 level1까지 탐색하는 방식 고려
- 두 방식의 삽입 성능 비교 실험이 필요함.

| 전략 | 설명 |
| --- | --- |
| A | node_level부터 탐색  |
| B | max_level 부터 탐색 |

### 3.8.2. 구현 방법

```cpp
// 전략 A – node_level부터 탐색
template<typename Key>
void SkipList<Key>::Insert(const Key& key) {
    int node_level = RandomLevel(); // 새 노드의 레벨 결정
    Node* node = new Node(key, node_level); // 새 노드 생성
    Node* p = head; // 탐색 시작
    std::vector<Node*> update(max_level, nullptr); // 삽입 위치 저장용 벡터

    // node_level - 1부터 하향식 탐색 수행
    for (int level = node_level - 1; level >= 0; level--) {
        while (p->next[level] != nullptr && compare_(key, p->next[level]->key) > 0) {
            p = p->next[level];
        }
        update[level] = p;
    }

    // Level 0에서 중복 검사
    if (update[0]->next[0] != nullptr && compare_(key, update[0]->next[0]->key) == 0) {
        delete node;
        return;
    }

    // 삽입 수행
    for (int i = 0; i < node_level; ++i) {
        node->next[i] = update[i]->next[i];
        update[i]->next[i] = node;
    }
}
```

```cpp
// 전략B - max_level - 1부터 탐색
template<typename Key>
void SkipList<Key>::Insert(const Key& key) {
    int node_level = RandomLevel(); // 새 노드의 레벨 결정
    Node* node = new Node(key, node_level); // 새 노드 생성
    Node* p = head;
    std::vector<Node*> update(max_level, nullptr); // 삽입 위치 저장용 벡터

    // max_level - 1부터 모든 레벨을 내려가며 삽입 위치 결정
    for (int level = max_level - 1; level >= 0; level--) {
        while (p->next[level] != nullptr && compare_(key, p->next[level]->key) > 0) {
            p = p->next[level];
        }
        if (level < node_level) {
            update[level] = p;
        }
    }

    // Level 0에서 중복 검사
    if (update[0] && update[0]->next[0] != nullptr && compare_(key, update[0]->next[0]->key) == 0) {
        delete node;
        return;
    }

    // 삽입 수행
    for (int i = 0; i < node_level; ++i) {
        node->next[i] = update[i]->next[i];
        update[i]->next[i] = node;
    }
}
```

### 3.8.3. 실험 환경

```cpp
#include <chrono>
#include <iostream>
#include <random>
#include "skiplist.h"

typedef uint64_t Key;
typedef std::chrono::high_resolution_clock Clock;

void CompareInsertStartLevelStrategies(int data_size, bool use_strategy_a) {
    const int repeat = 100;
    double total_time = 0;

    for (int i = 0; i < repeat; ++i) {
        SkipList<Key> sl;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<Key> dist(1, data_size);

        auto start = Clock::now();

        for (int j = 0; j < data_size; ++j) {
            Key k = dist(gen);
            int node_level = sl.RandomLevel();
            auto* node = new SkipList<Key>::Node(k, node_level);
            auto* p = sl.head;
            std::vector<SkipList<Key>::Node*> update(sl.max_level, nullptr);

            if (use_strategy_a) {
                // 전략 A: node_level - 1부터 시작
                }
            } else {
                // 전략 B: max_level - 1부터 시작
            }

            if (update[0] && update[0]->next[0] != nullptr && compare_(k, update[0]->next[0]->key) == 0) {
                delete node;
                continue;
            }

            for (int i = 0; i < node_level; ++i) {
                node->next[i] = update[i]->next[i];
                update[i]->next[i] = node;
            }
        }

        auto end = Clock::now();
        total_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    }

    double avg_time = total_time / repeat / 1000.0;
    std::cout << (use_strategy_a ? "전략 A (node_level부터 탐색)" : "전략 B (항상 max_level부터 탐색)");
    std::cout << " 평균 삽입 시간: " << avg_time << " µs" << std::endl;
}

int main() {
    int data_size = 10000;
    std::cout << "data_size: " << data_size <<std::endl;
    CompareInsertStartLevelStrategies(data_size, true);  // 전략 A
    CompareInsertStartLevelStrategies(data_size, false); // 전략 B
    return 0;
}
```

| 항목 | 내용 |
| --- | --- |
| 데이터 개수 | 10,000 |
| 반복 횟수 | 100회 |
| 측정 대상 | 노드 삽입 시간 |
| 기기 | MacBook Air (Apple Silicon) |
| 컴파일러 | g++ -std=c++17 |
| 측정 단위 | 평균 삽입 시간 (마이크로초, µs) |

### 3.8.4 실험 결과

| 실행 | 전략 A (node_level부터 탐색) | 전략 B (항상 max_level부터 탐색) | 차이 |
| --- | --- | --- | --- |
| 1회 | 117,130 µs | 8,537.84 µs | -108,592.16 µs |
| 2회 | 117,850 µs | 8,467.91 µs | -109,382.09 µs |
| 3회 | 118,653 µs | 8,466.50 µs | -110,186.5 µs |
- max_level부터 탐색을 시작하는 전략(전략 B)이 삽입 성능 면에서 이점을 가짐을 알 수 있음
- 따라서 insert 메소드에서 max_level부터 탐색하는 방식으로 구현하는 것이 훨씬 효율적임을 확인할 수 있음

---

# 4. Contains 구현 과정

## 4.1. First Thinking

- 가장 높은 레벨부터 시작해, 현재 key보다 작은 노드를 따라가며 내려감
- 현재 key보다 크거나 같은 값을 발견하면 그 값과 비교하여 탐색을 종료
- 탐색 중 key와 동일한 값을 발견하면 true, 끝까지 찾지 못하면 false 반환

```cpp
template<typename Key>
bool SkipList<Key>::Contains(const Key& key) const {
    Node* p = head;
    for (int level = max_level - 1; level >= 0; level--) {
        while (p->next[level] != nullptr && compare_(key, p->next[level]->key) > 0) {
            p = p->next[level];
        }
        if (p->next[level] != nullptr && compare_(key, p->next[level]->key) == 0) {
            return true;
        }
    }
    return false;
}
```

---

# 5. Delete 구현 과정

### 5.1. First Thinking

- SkipList 구조상 각 레벨마다 해당 노드를 가리키는 포인터들이 존재할 수 있음
- 실제 메모리 해제(`delete`)는 마지막으로 1회만 수행됨
- 삭제 대상 노드를 임시 포인터(`Node* d`)에 저장하고, 각 레벨에서 이 노드를 건너뛰도록 전후 노드를 연결

```cpp
template<typename Key>
bool SkipList<Key>::Delete(const Key& key) const {
    Node* p = head;
    Node* d = nullptr; // 삭제해야 하는 노드 저장

    // 각 레벨에서 삭제 대상 노드의 연결 해제
    for (int level = max_level - 1; level >= 0; level--) {
        while (p->next[level] != nullptr) {
            int cmp = compare_(key, p->next[level]->key);
            if (cmp == 0) {
                d = p->next[level]; // 삭제 대상 노드 기억
                p->next[level] = d->next[level]; // 연결 끊기
            } else if (cmp > 0) {
                p = p->next[level]; // 다음 노드로 이동
            } else {
                break; // 해당 레벨에서는 key를 초과한 노드 → skip
            }
        }
    }

    if (d != nullptr) { // 삭제할 노드를 찾았다면
        delete d; // 실제 삭제 (delete는 단 한 번만 수행)
        return true;
    }
    return false;
}
```

---

# 6. Scan 구현 과정

## 6.1. First Thinking

- Scan 함수는 특정 key부터 시작하여 `scan_num` 개의 값을 순차적으로 수집 후 리턴하는 함수

## 6.2. [실험] Scan 탐색 전략에 대한 성능 비교 실험

### 6.2.1  실험 목적

key 이상 노드 탐색에 있어 두 가지 전략을 비교할 수 있다.

- **[전략 A]**: level 0에서 탐색
- **[전략 B]**: 상위 레벨부터 순차적으로 탐색 후, level 0에서 수집

본 실험에서는 두 탐색 전략에 대해 실제 데이터를 이용해 **성능 차이**를 분석해보고자 한다. 

### 6.2.2 구현 방법

```cpp
// 전략 A : Level 0에서 선형 탐색
template<typename Key>
std::vector<Key> SkipList<Key>::Scan(const Key& key, const int scan_num) {
    Node* p = head;
    int cnt = scan_num;
    std::vector<Key> result = {};
    while (p->next[0] != nullptr && compare_(key, p->next[0]->key) > 0) {
        p = p->next[0];
    }
    while (cnt > 0 && p->next[0] != nullptr) {
        result.push_back(p->next[0]->key);
        p = p->next[0];
        cnt -= 1;
    }
    return result;
}
```

```cpp
// 전략 B : Top-down 탐색 후 Level 0에서 수집
std::vector<Key> SkipList<Key>::Scan(const Key& key, const int scan_num) {
    Node* p = head;
    int cnt = scan_num;
    std::vector<Key> result = {};

		// max_level - 1부터 시작
    for (int level = max_level - 1; level >= 0; level--) { 
        while (p->next[level] != nullptr && compare_(key, p->next[level]->key) > 0) {
            p = p->next[level];
        }
    }
    while (cnt > 0 && p->next[0] != nullptr) {
        result.push_back(p->next[0]->key);
        p = p->next[0];
        cnt --;
    }
    return result;
}
```

### 6.2.3 실험 환경

```cpp
#include <chrono>
#include <iostream>
#include <vector>
#include <random>
#include "skiplist.h"
#include <mutex>

using namespace std::chrono;

typedef high_resolution_clock Clock;
typedef uint64_t Key;

// 전략 A: Level 0 선형 탐색
template<typename Key>
std::vector<Key> ScanStrategyA(SkipList<Key>& sl, const Key& key, const int scan_num) {
    typename SkipList<Key>::Node* p = sl.head;
    int cnt = scan_num;
    std::vector<Key> result = {};

    while (p->next[0] != nullptr && compare_(key, p->next[0]->key) > 0) {
        p = p->next[0];
    }
    while (cnt > 0 && p->next[0] != nullptr) {
        result.push_back(p->next[0]->key);
        p = p->next[0];
        cnt -= 1;
    }
    return result;
}

// 전략 B: Top-down 탐색 후 level 0에서 수집
template<typename Key>
std::vector<Key> ScanStrategyB(SkipList<Key>& sl, const Key& key, const int scan_num) {
    typename SkipList<Key>::Node* p = sl.head;
    int cnt = scan_num;
    std::vector<Key> result = {};

    for (int level = sl.max_level - 1; level >= 0; level--) {
        while (p->next[level] != nullptr && compare_(key, p->next[level]->key) > 0) {
            p = p->next[level];
        }
    }
    while (cnt > 0 && p->next[0] != nullptr) {
        result.push_back(p->next[0]->key);
        p = p->next[0];
        cnt -= 1;
    }
    return result;
}

// Scan 전략별 100회 평균 시간 측정 함수
void CompareScanStrategies(int write_size, int scan_num) {
    const int repeat = 100;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<Key> key_gen(1, write_size);

    double total_time_A = 0;
    double total_time_B = 0;

    for (int i = 0; i < repeat; ++i) {
        SkipList<Key> sl;

        // 데이터 삽입
        for (int j = 1; j <= write_size; ++j) {
            sl.Insert(j);
        }

        Key key = key_gen(gen);

        // 전략 A 시간 측정
        auto start_a = Clock::now();
        ScanStrategyA(sl, key, scan_num);
        auto end_a = Clock::now();
        total_time_A += duration_cast<nanoseconds>(end_a - start_a).count();

        // 전략 B 시간 측정
        auto start_b = Clock::now();
        ScanStrategyB(sl, key, scan_num);
        auto end_b = Clock::now();
        total_time_B += duration_cast<nanoseconds>(end_b - start_b).count();
    }

    double avg_time_A = total_time_A / repeat / 1000.0; // us
    double avg_time_B = total_time_B / repeat / 1000.0; // us

    std::cout << "==== [Scan 전략 성능 비교: 평균 (" << repeat << "회)] ====\n";
    std::cout << "데이터 크기: " << write_size << ", scan_num: " << scan_num << "\n";
    std::cout << "전략 A (Level 0 선형 탐색) : " << avg_time_A << " µs\n";
    std::cout << "전략 B (Top-down 탐색 후 Level 0) : " << avg_time_B << " µs\n";
    std::cout << "차이 (A - B) : " << avg_time_A - avg_time_B << " µs\n";
}

int main() {
    int write_size = 10000; // 삽입할 키 수
    int scan_num = 1000;    // Scan 크기
    CompareScanStrategies(write_size, scan_num);
    return 0;
}

```

| 항목 | 세부 내용 |
| --- | --- |
| **실험 목적** | Scan의 탐색 전략 비교
- **[전략 A]**: level 0에서 탐색 시작 
- **[전략 B]**: 상위 레벨부터 순차적으로 탐색 후, level 0에서 수집 |
| **측정 항목** | Scan 함수 평균 수행 시간 (μs 단위) |
| **반복 횟수 (repeat)** | 100회 |
| **데이터 크기 (write_size)** | 10,000 |
| **Scan 범위 (scan_num)** | 1,000 |
| **삽입 데이터** | 1부터 write_size까지 연속 정수 삽입 |
| **Scan 대상 key** | [1, write_size] 범위의 균등분포에서 난수 생성 |
| **난수 생성기** | std::mt19937 + std::uniform_int_distribution |
| **시간 측정 방식** | std::chrono::high_resolution_clock 사용 |
| **CPU** | Apple M2 8-core |
| **운영체제 (OS)** | macOS Sonoma 14.x |
| **컴파일 옵션** | -std=c++11 |
| **랜덤 시드** | 기본 시스템 난수 (std::random_device), srand(0) 미사용 |

### 6.2.4 실험 결과

> ==== [Scan 전략 성능 비교: 평균 (100회)] ====
데이터 크기: 10000, scan_num: 1000
전략 A (Level 0 선형 탐색) : 83.803 µs
전략 B (Top-down 탐색 후 Level 0) : 38.2958 µs
차이 (A - B) : 45.5071 µs
> 
- 전략 B (Top-down 탐색 후 Level 0 탐색)이 전략 A보다 **약 45.5μs 더 빠른 성능**을 보임.
- 특히 데이터 크기 및 scan 범위가 커질수록, 레벨 구조를 활용한 탐색 전략(전략 B)이 **성능상 이점**을 가질 가능성이 높음.
- 따라서 Scan 구현 시 **전략 B 방식**을 채택하는 것이 성능 향상 측면에서 더 바람직하다는 결론을 내림.

---

# 7. RandomLevel 구현 과정

### 7.1. First Thinking

- 레벨은 1부터 max_level까지 가질 수 있으며, 이 값은 확률에 따라 결정.
- 레벨 1부터 시작하고, 랜덤 값이 일정 확률보다 작을 경우 레벨을 1 증가시키는 로직으로 구성.

### 7.2. Error_srand() 설정

### (1) 현상

- 벤치마크 실행 시, SkipList의 구성이 동일한 결과를 출력.

### (2) 원인

- rand() 함수는 동일한 시드값을 사용하면 매번 동일한 난수를 반환 함
- 이를 해결하기 위해, 프로그램 실행 시마다 랜덤 시드 설정이 필요함.

### (3) 해결방안

- srand(time(NULL));을 main() 함수 처음에 추가.

---

# 8 .  파라미터 실험 및 성능 분석

## 8.1 파리미터 실험 개요

- max_level : SkipList에서 가능한 최대 레벨 수
- probability : 새로운 노드가 가질 수 있는 최대 레벨을 확률적으로 결정.

각 파라미터에 대해 쓰기 성능과 검색 성능 (lookup)을 비교함.

## 8.2 실험 1_max_level의 성능 분석

### 8.2.1. 쓰기 성능 측정

| 데이터 크기 | **max_level = 16** (µs) | **max_level = 160** (µs) | **max_level = 1600** (µs) |
| --- | --- | --- | --- |
| 1000 | 7368.08 | 10454.83 | 37951.33 |
| 10000 | 243257.67 | 247997.17 | 405732.34 |
| 100000 | 22222846.00 | 21785118.00 | 23309786.00 |
- max_level이 커짐에 따라 쓰기 성능(삽입 시간)은 점진적으로 증가 함. 이는 더 많은 레벨을 가지고 있기 때문에 노드를 삽입하는 데 더 많은 시간이 소요되기 때문.
- max_level = 1600의 경우, max_level = 16과 비교했을 때 삽입 시간이 약 5배 정도 더 오래 걸림

### 8.2.2. 검색 성능 측정

| 데이터 크기 | **max_level = 16** (µs) | **max_level = 160** (µs) | **max_level = 1600** (µs) |
| --- | --- | --- | --- |
| 1000 | 569.96 | 1481.75 | 7797.42 |
| 10000 | 2323.62 | 7969.46 | 54630.71 |
| 100000 | 17244.17 | 38840.96 | 276408.00 |
- max_level이 커질수록 검색 성능은 증가하는 경향을 보임.
- 높은 max_level 값은 SkipList가 더 많은 레벨을 가지게 되어 더 적은 탐색 횟수로 노드를 찾을 수 있기 때문.

## 8.3. 실험2_probability의 성능 분석

### 8.3.1. 쓰기 성능 측정

| 데이터 크기 | **probability = 0.5** (µs) | **probability = 0.2** (µs) | **probability = 0.8** (µs) |
| --- | --- | --- | --- |
| 1000 | 7368.08 | 8725.62 | 217014.17 |
| 10000 | 243257.67 | 287940.88 | 217014.17 |
| 100000 | 22222846.00 | 26804892.00 | 19946046.00 |
- probability 값이 낮을수록 삽입 성능이 우수함.
- 낮은 확률로 생성된 max_level이 상대적으로 낮기 때문에, 노드를 삽입하는 데 시간이 덜 소요되기 때문.

### 8.3.2. 검색 성능 측정

| 데이터 크기 | **probability = 0.5** (µs) | **probability = 0.2** (µs) | **probability = 0.8** (µs) |
| --- | --- | --- | --- |
| 1000 | 569.96 | 561.67 | 12734.88 |
| 10000 | 2323.62 | 2947.62 | 12734.88 |
| 100000 | 17244.17 | 15569.67 | 396897.50 |
- probability 값이 높을수록 검색 성능이 우수함.
- 확률이 높으면 max_level이 커져서 더 적은 레벨을 탐색하며, 더 효율적으로 검색할 수 있기 때문.

## 8.4 결론

- max_level이 증가할수록 검색 성능이 향상되지만, 쓰기 성능은 감소하는 경향이 있음.
- probability가 낮을수록 삽입 성능은 우수하지만, 검색 성능은 나빠지는 경향이 있음.
- probability 값이 높을수록 더 많은 레벨이 생성되어 검색 성능은 좋아지지만, 삽입 성능은 저하됨.

---
