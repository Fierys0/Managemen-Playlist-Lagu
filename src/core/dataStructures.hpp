#pragma once
#include <cstddef>
#include <vector>

template <typename T> class DoublyLinkedList {
public:
  // [LINKED LIST] Node — elemen dasar senarai berantai ganda
  struct Node {
    T data;
    Node *next{nullptr};
    Node *prev{nullptr};
    Node(const T &val) : data(val) {}
    Node(T &&val) : data(std::move(val)) {}
  };

  DoublyLinkedList() = default;
  ~DoublyLinkedList() { clear(); }

  // [LINKED LIST] salin seluruh node satu per satu
  DoublyLinkedList(const DoublyLinkedList &other) {
    Node *cur = other.m_head;
    while (cur) {
      pushBack(cur->data);
      cur = cur->next;
    }
  }

  // [LINKED LIST] Copy assignment operator
  DoublyLinkedList &operator=(const DoublyLinkedList &other) {
    if (this != &other) {
      clear();
      Node *cur = other.m_head;
      while (cur) {
        pushBack(cur->data);
        cur = cur->next;
      }
    }
    return *this;
  }

  // [LINKED LIST] Move constructor — pindahkan kepemilikan node
  DoublyLinkedList(DoublyLinkedList &&other) noexcept
      : m_head(other.m_head), m_tail(other.m_tail), m_size(other.m_size) {
    other.m_head = nullptr;
    other.m_tail = nullptr;
    other.m_size = 0;
  }

  // [LINKED LIST] Move assignment operator
  DoublyLinkedList &operator=(DoublyLinkedList &&other) noexcept {
    if (this != &other) {
      clear();
      m_head = other.m_head;
      m_tail = other.m_tail;
      m_size = other.m_size;
      other.m_head = nullptr;
      other.m_tail = nullptr;
      other.m_size = 0;
    }
    return *this;
  }

  // [LINKED LIST] pushBack — Tambahkan elemen baru di akhir senarai
  // Kompleksitas: O(1) karena kita menyimpan pointer ke tail
  void pushBack(const T &val) {
    Node *node = new Node(val);
    if (!m_tail) {
      m_head = m_tail = node;
    } else {
      node->prev = m_tail;
      m_tail->next = node;
      m_tail = node;
    }
    ++m_size;
  }

  // [LINKED LIST] pushBack (move) — Tambahkan elemen baru dengan memindahkan
  // data
  void pushBack(T &&val) {
    Node *node = new Node(std::move(val));
    if (!m_tail) {
      m_head = m_tail = node;
    } else {
      node->prev = m_tail;
      m_tail->next = node;
      m_tail = node;
    }
    ++m_size;
  }

  // [LINKED LIST] remove — Hapus node tertentu dari senarai
  // Kompleksitas: O(1) karena kita langsung mengubah pointer prev/next
  void remove(Node *node) {
    if (!node)
      return;
    if (node->prev)
      node->prev->next = node->next;
    else
      m_head = node->next;
    if (node->next)
      node->next->prev = node->prev;
    else
      m_tail = node->prev;
    delete node;
    --m_size;
  }

  // [LINKED LIST] clear — Hapus semua node dan bebaskan memori
  void clear() {
    Node *cur = m_head;
    while (cur) {
      Node *next = cur->next;
      delete cur;
      cur = next;
    }
    m_head = m_tail = nullptr;
    m_size = 0;
  }

  // [LINKED LIST] Akses ukuran dan status kosong
  int size() const { return m_size; }
  bool empty() const { return m_size == 0; }

  Node *head() const { return m_head; }
  Node *tail() const { return m_tail; }

  // [LINKED LIST] toVector — Konversi senarai ke std::vector
  // Digunakan saat serialisasi data ke JSON atau saat membutuhkan akses indeks
  std::vector<T> toVector() const {
    std::vector<T> result;
    result.reserve(m_size);
    Node *cur = m_head;
    while (cur) {
      result.push_back(cur->data);
      cur = cur->next;
    }
    return result;
  }

  // [LINKED LIST] Iterator — mendukung range-based for loop
  struct Iterator {
    Node *node;
    Iterator(Node *n) : node(n) {}
    T &operator*() { return node->data; }
    const T &operator*() const { return node->data; }
    T *operator->() { return &node->data; }
    Iterator &operator++() {
      node = node->next;
      return *this;
    }
    bool operator!=(const Iterator &other) const { return node != other.node; }
    bool operator==(const Iterator &other) const { return node == other.node; }
  };

  struct ConstIterator {
    const Node *node;
    ConstIterator(const Node *n) : node(n) {}
    const T &operator*() const { return node->data; }
    const T *operator->() const { return &node->data; }
    ConstIterator &operator++() {
      node = node->next;
      return *this;
    }
    bool operator!=(const ConstIterator &other) const {
      return node != other.node;
    }
    bool operator==(const ConstIterator &other) const {
      return node == other.node;
    }
  };

  Iterator begin() { return Iterator(m_head); }
  Iterator end() { return Iterator(nullptr); }
  ConstIterator begin() const { return ConstIterator(m_head); }
  ConstIterator end() const { return ConstIterator(nullptr); }

private:
  Node *m_head{nullptr};
  Node *m_tail{nullptr};
  int m_size{0};
};

// =============================================================================
// [CIRCULAR LINKED LIST] CircularLinkedList — Senarai Berantai Melingkar Ganda
// =============================================================================
// Struktur data senarai berantai melingkar ganda (Circular Doubly Linked List).
// Node terakhir menunjuk kembali ke head, dan head menunjuk kembali ke tail.
// Pointer 'current' melacak posisi pemutaran aktif.
// Digunakan untuk antrean pemutaran lagu agar navigasi Next/Prev
// secara otomatis melingkar tanpa perlu pengecekan batas indeks.
// =============================================================================
template <typename T> class CircularLinkedList {
public:
  // [CIRCULAR LINKED LIST] Node — elemen dasar senarai melingkar
  struct Node {
    T data;
    Node *next{nullptr};
    Node *prev{nullptr};
    Node(const T &val) : data(val) {}
    Node(T &&val) : data(std::move(val)) {}
  };

  CircularLinkedList() = default;
  ~CircularLinkedList() { clear(); }

  // [CIRCULAR LINKED LIST] Copy constructor
  CircularLinkedList(const CircularLinkedList &other) {
    if (other.m_size == 0)
      return;
    // Salin semua node dari senarai sumber
    Node *src = other.m_head;
    int currentIdx = other.getCurrentIndex();
    for (int i = 0; i < other.m_size; ++i) {
      pushBack(src->data);
      src = src->next;
    }
    // Posisikan current ke indeks yang sama
    m_current = m_head;
    for (int i = 0; i < currentIdx && m_current; ++i) {
      m_current = m_current->next;
    }
  }

  // [CIRCULAR LINKED LIST] Copy assignment operator
  CircularLinkedList &operator=(const CircularLinkedList &other) {
    if (this != &other) {
      clear();
      if (other.m_size == 0)
        return *this;
      Node *src = other.m_head;
      int currentIdx = other.getCurrentIndex();
      for (int i = 0; i < other.m_size; ++i) {
        pushBack(src->data);
        src = src->next;
      }
      m_current = m_head;
      for (int i = 0; i < currentIdx && m_current; ++i) {
        m_current = m_current->next;
      }
    }
    return *this;
  }

  // [CIRCULAR LINKED LIST] Move constructor
  CircularLinkedList(CircularLinkedList &&other) noexcept
      : m_head(other.m_head), m_current(other.m_current), m_size(other.m_size) {
    other.m_head = nullptr;
    other.m_current = nullptr;
    other.m_size = 0;
  }

  // [CIRCULAR LINKED LIST] Move assignment operator
  CircularLinkedList &operator=(CircularLinkedList &&other) noexcept {
    if (this != &other) {
      clear();
      m_head = other.m_head;
      m_current = other.m_current;
      m_size = other.m_size;
      other.m_head = nullptr;
      other.m_current = nullptr;
      other.m_size = 0;
    }
    return *this;
  }

  // [CIRCULAR LINKED LIST] pushBack — Tambahkan elemen baru di akhir lingkaran
  // Setelah ditambahkan, node baru menjadi prev dari head dan next dari tail
  // lama
  void pushBack(const T &val) {
    Node *node = new Node(val);
    if (!m_head) {
      // Senarai kosong: node baru menunjuk ke dirinya sendiri (melingkar)
      m_head = node;
      node->next = node;
      node->prev = node;
      m_current = node;
    } else {
      // Sisipkan sebelum head (di akhir lingkaran)
      Node *tail = m_head->prev;
      tail->next = node;
      node->prev = tail;
      node->next = m_head;
      m_head->prev = node;
    }
    ++m_size;
  }

  // [CIRCULAR LINKED LIST] clear — Hapus semua node dan putuskan lingkaran
  void clear() {
    if (!m_head) {
      m_size = 0;
      m_current = nullptr;
      return;
    }
    // Putuskan lingkaran terlebih dahulu agar bisa iterasi linear
    Node *tail = m_head->prev;
    if (tail)
      tail->next = nullptr;

    Node *cur = m_head;
    while (cur) {
      Node *next = cur->next;
      delete cur;
      cur = next;
    }
    m_head = nullptr;
    m_current = nullptr;
    m_size = 0;
  }

  // [CIRCULAR LINKED LIST] Akses ukuran dan status kosong
  int size() const { return m_size; }
  bool empty() const { return m_size == 0; }

  // [CIRCULAR LINKED LIST] getCurrent — Kembalikan data pada posisi pemutaran
  // aktif
  T *getCurrent() { return m_current ? &m_current->data : nullptr; }
  const T *getCurrent() const { return m_current ? &m_current->data : nullptr; }

  // [CIRCULAR LINKED LIST] moveNext — Pindahkan pointer current ke node
  // berikutnya Karena senarai melingkar, setelah node terakhir akan kembali ke
  // node pertama
  void moveNext() {
    if (m_current)
      m_current = m_current->next;
  }

  // [CIRCULAR LINKED LIST] movePrev — Pindahkan pointer current ke node
  // sebelumnya Karena senarai melingkar, sebelum node pertama akan kembali ke
  // node terakhir
  void movePrev() {
    if (m_current)
      m_current = m_current->prev;
  }

  // [CIRCULAR LINKED LIST] resetToHead — Kembalikan pointer current ke head
  void resetToHead() { m_current = m_head; }

  // [CIRCULAR LINKED LIST] setCurrent — Set current ke node tertentu
  // berdasarkan data
  void setCurrentByData(const T &val) {
    if (!m_head)
      return;
    Node *cur = m_head;
    for (int i = 0; i < m_size; ++i) {
      if (cur->data.filePath == val.filePath) {
        m_current = cur;
        return;
      }
      cur = cur->next;
    }
  }

  // [CIRCULAR LINKED LIST] getCurrentIndex — Hitung posisi current relatif
  // terhadap head Digunakan untuk menampilkan "Lagu X dari Y" di layar
  // pemutaran
  int getCurrentIndex() const {
    if (!m_head || !m_current)
      return -1;
    int idx = 0;
    Node *cur = m_head;
    while (cur != m_current) {
      ++idx;
      cur = cur->next;
      if (cur == m_head)
        return -1; // Tidak ditemukan (seharusnya tidak terjadi)
    }
    return idx;
  }

  // [CIRCULAR LINKED LIST] getNextN — Ambil N lagu berikutnya dari posisi
  // current Digunakan untuk menampilkan daftar "Selanjutnya / Next Up" di layar
  // pemutaran
  std::vector<const T *> getNextN(int n) const {
    std::vector<const T *> result;
    if (!m_current || m_size <= 1)
      return result;
    Node *cur = m_current->next;
    for (int i = 0; i < n && i < m_size - 1; ++i) {
      result.push_back(&cur->data);
      cur = cur->next;
    }
    return result;
  }

private:
  Node *m_head{nullptr};
  Node *m_current{nullptr};
  int m_size{0};
};
