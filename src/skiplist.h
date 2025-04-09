#include <stdlib.h>
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <xmmintrin.h>
#include <immintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include <bit>
#include <functional>
#include <mutex>
#include <vector>

#include <atomic>

typedef std::chrono::high_resolution_clock Clock;

// Key is an 8-byte integer
typedef uint64_t Key;

int compare_(const Key& a, const Key& b) {
    if (a < b) {
        return -1;
    } else if (a > b) {
        return +1;
    } else {
        return 0;
    }
}

template<typename Key>
class SkipList {
   private:
    struct Node;

   public:
    SkipList(int max_level = 16, float probability = 0.5);

    void Insert(const Key& key); // Insertion function (to be implemented by students)
    bool Contains(const Key& key) const; // Lookup function (to be implemented by students)
    std::vector<Key> Scan(const Key& key, const int scan_num); // Range query function (to be implemented by students)
    bool Delete(const Key& key) const; // Delete function (to be implemented by students)

    void Print() const;

   private:
    int RandomLevel(); // Generates a random level for new nodes (to be implemented by students)

    Node* head; // Head node (starting point of the SkipList)
    int max_level; // Maximum level in the SkipList
    float probability; // Probability factor for level increase
};

// SkipList Node structure
template<typename Key>
struct SkipList<Key>::Node {
    Key key;
    std::vector<Node*> next; // Pointer array for multiple levels

    // Constructor for Node
    Node(Key key, int level);
};

template<typename Key>
SkipList<Key>::Node::Node(Key key, int level)
    : key(key), next(level, nullptr) {}

template<typename Key>
int SkipList<Key>::RandomLevel() {
    int level = 1; // 기본 level
    while (((float)rand() / RAND_MAX) < probability && level < max_level) { // 확률 값에 따라 level 증가
        level++;
    }
    return level; // 해당 노드의 레벨 리턴
}

template<typename Key>
SkipList<Key>::SkipList(int max_level, float probability)
    : max_level(max_level), probability(probability) {
    head = new Node(Key(), max_level); // define head node to key = 1
}

template<typename Key>
void SkipList<Key>::Insert(const Key& key) {
    int node_level = RandomLevel(); // 새로운 노드에 할당할 레벨을 확률적으로 결정
    Node* node = new Node(key, node_level); // 해당 레벨을 가진 노드 생성
    Node* p = head; // 포인터 초기화
    std::vector<Node*> update(max_level, nullptr); // 각 레벨에서 삽입 위치 직전 노드(p)를 저장

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

template<typename Key>
bool SkipList<Key>::Delete(const Key& key) const {
    Node* p = head;       // 탐색 시작 지점
    Node* d = nullptr;    // 실제로 삭제할 노드를 저장할 포인터

    // 가장 높은 레벨부터 아래로 내려가며 탐색
    for (int level = max_level - 1; level >= 0; level--) {
        while (p->next[level] != nullptr) {
            int cmp = compare_(key, p->next[level]->key); // key와 다음 노드의 key 비교

            if (cmp == 0) { // 삭제할 key를 찾은 경우 (key와 동일한 키를 가진 노드를 찾은 경우)
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
        return true;     // 삭제 성공 시 true 리턴
    }

    return false; // 삭제 대상 key가 없을 경우 false 리턴
}


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

template<typename Key>
void SkipList<Key>::Print() const {
  std::cout << "SkipList Structure:\n";
  for (int level = max_level - 1; level >= 0; --level) {
    Node* node = head->next[level];
    std::cout << "Level " << level << ": ";
    while (node != nullptr) {
      std::cout << node->key << " ";
      node = node->next[level];
    }
    std::cout << "\n";
  }
}
