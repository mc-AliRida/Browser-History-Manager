#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>

using namespace std;

// ============================= DATA STRUCTURES =============================

struct related_Pages {
    int R_ID;
    string URL;
    string title;
    related_Pages* next;
};

struct Page {
    int P_ID;
    string URL;
    string title;
    time_t visitedAt;
    Page* prev, * next;
    related_Pages* rp_head;
};

struct pages_History {
    Page* head, * tail;
};

struct bookMarkNode {
    Page* page;
    bool favorite;
    int visitCount;
    time_t lastVisited;
    bookMarkNode* prev, * next;
};

struct bookMarkList {
    bookMarkNode* head, * tail;
};

// ========================= INITIALIZATION =========================

void initializeHistory(pages_History* history) {
    history->head = nullptr;
    history->tail = nullptr;
}

void initializeBookmarkList(bookMarkList* bookmarks) {
    bookmarks->head = nullptr;
    bookmarks->tail = nullptr;
}

// ========================= STRING HELPERS =========================

string trim(const string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

// ========================= DATE/TIME HELPERS =========================

time_t parseDateTime(const string& dateTimeStr) {
    struct tm timeInfo = {};
    istringstream iss(dateTimeStr);
    char delimiter;

    iss >> timeInfo.tm_mday >> delimiter
        >> timeInfo.tm_mon >> delimiter
        >> timeInfo.tm_year >> delimiter
        >> timeInfo.tm_hour >> delimiter
        >> timeInfo.tm_min;

    timeInfo.tm_mon -= 1;
    timeInfo.tm_year -= 1900;
    timeInfo.tm_sec = 0;

    return mktime(&timeInfo);
}

string formatDateTime(time_t timeValue) {
    struct tm timeInfo;

#ifdef _WIN32
    localtime_s(&timeInfo, &timeValue);
#else
    localtime_r(&timeValue, &timeInfo);
#endif

    char buffer[20];
    strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M", &timeInfo);
    return string(buffer);
}

// ========================= VALIDATION FUNCTIONS =========================

bool isValidURL(const string& url) {
    if (url.find("https://") != 0) {
        return false;
    }

    if (url.find(".com") != string::npos ||
        url.find(".org") != string::npos ||
        url.find(".net") != string::npos ||
        url.find(".edu") != string::npos) {
        return true;
    }

    return false;
}

bool isValidTitle(const string& title) {
    if (title.empty()) return false;

    for (char c : title) {
        if (!isalpha(c) && c != ' ') {
            return false;
        }
    }
    return true;
}

string getValidURL() {
    string url;
    while (true) {
        cout << "URL: ";
        cin >> url;

        if (isValidURL(url)) {
            return url;
        }
        cout << "✗ Invalid URL format!" << endl;
    }
}

string getValidTitle() {
    string title;
    while (true) {
        cout << "Title: ";
        cin.ignore();
        getline(cin, title);

        if (isValidTitle(title)) {
            return title;
        }
        cout << "✗ Invalid title!" << endl;
    }
}

// ========================= CREATE NODE HELPERS =========================

Page* createNewPage(int P_ID, const string& url, const string& title, time_t visitedAt) {
    Page* newPage = new Page;
    newPage->P_ID = P_ID;
    newPage->URL = url;
    newPage->title = title;
    newPage->visitedAt = visitedAt;
    newPage->prev = nullptr;
    newPage->next = nullptr;
    newPage->rp_head = nullptr;
    return newPage;
}

Page* createNewPage(int P_ID, const string& url, const string& title) {
    return createNewPage(P_ID, url, title, time(nullptr));
}

related_Pages* createRelatedPage(int R_ID, const string& url, const string& title) {
    related_Pages* newRelated = new related_Pages;
    newRelated->R_ID = R_ID;
    newRelated->URL = url;
    newRelated->title = title;
    newRelated->next = nullptr;
    return newRelated;
}

bookMarkNode* createBookmarkNode(Page* page, bool favorite, int visitCount, time_t lastVisited) {
    bookMarkNode* newBookmark = new bookMarkNode;
    newBookmark->page = page;
    newBookmark->favorite = favorite;
    newBookmark->visitCount = visitCount;
    newBookmark->lastVisited = lastVisited;
    newBookmark->prev = nullptr;
    newBookmark->next = nullptr;
    return newBookmark;
}

// ========================= ADD TO LIST HELPERS =========================

void addPageToHistory(pages_History* history, Page* page) {
    if (history->head == nullptr) {
        history->head = history->tail = page;
    }
    else {
        history->tail->next = page;
        page->prev = history->tail;
        history->tail = page;
    }
}

void addRelatedToPage(Page* page, related_Pages* related) {
    if (page->rp_head == nullptr) {
        page->rp_head = related;
    }
    else {
        related_Pages* current = page->rp_head;
        while (current->next != nullptr) {
            current = current->next;
        }
        current->next = related;
    }
}

void addBookmarkToList(bookMarkList* bookmarks, bookMarkNode* bookmark) {
    if (bookmarks->head == nullptr) {
        bookmarks->head = bookmarks->tail = bookmark;
    }
    else {
        bookmarks->tail->next = bookmark;
        bookmark->prev = bookmarks->tail;
        bookmarks->tail = bookmark;
    }
}

// ========================= SEARCH HELPERS =========================

Page* findPageByURL(pages_History* history, const string& url) {
    Page* current = history->head;
    while (current != nullptr) {
        if (current->URL == url) {
            return current;
        }
        current = current->next;
    }
    return nullptr;
}

bool isBookmarked(bookMarkList* bookmarks, Page* page) {
    bookMarkNode* current = bookmarks->head;
    while (current != nullptr) {
        if (current->page == page) {
            return true;
        }
        current = current->next;
    }
    return false;
}

// ========================= ID GENERATION =========================

int generateNextPageID(pages_History* history) {
    if (history->head == nullptr) {
        return 101;
    }
    return history->tail->P_ID + 1;
}

int calculateRelatedPageStartID(int P_ID) {
    return (P_ID - 101) * 100 + 501;
}

// =================== FUNCTION 1: PARSE INPUT FILE ===================

bool parseInputFile(const string& filename, pages_History* history, bookMarkList* bookmarks) {
    ifstream inputFile(filename);

    if (!inputFile.is_open()) {
        cout << "File not found. Starting with empty history." << endl;
        return false;
    }

    string line;
    Page* currentPage = nullptr;
    int pagesCount = 0, relatedCount = 0, bookmarksCount = 0;

    while (getline(inputFile, line)) {
        if (line.empty()) continue;

        char firstChar = line[0];

        if (firstChar == '-') {
            istringstream iss(line.substr(1));
            string P_ID_str, url, title, dateTime;

            getline(iss, P_ID_str, ',');
            getline(iss, url, ',');
            getline(iss, title, ',');
            getline(iss, dateTime);

            int P_ID = stoi(trim(P_ID_str));
            time_t visitedAt = parseDateTime(trim(dateTime));

            currentPage = createNewPage(P_ID, trim(url), trim(title), visitedAt);
            addPageToHistory(history, currentPage);
            pagesCount++;
        }
        else if (firstChar == '*') {
            if (currentPage == nullptr) continue;

            istringstream iss(line.substr(1));
            string R_ID_str, url, title;

            getline(iss, R_ID_str, ',');
            getline(iss, url, ',');
            getline(iss, title);

            int R_ID = stoi(trim(R_ID_str));
            related_Pages* related = createRelatedPage(R_ID, trim(url), trim(title));
            addRelatedToPage(currentPage, related);
            relatedCount++;
        }
        else if (firstChar == '#') {
            if (currentPage == nullptr) continue;

            istringstream iss(line.substr(1));
            string yesNo, visitCount_str, dateTime;

            getline(iss, yesNo, ',');
            getline(iss, visitCount_str, ',');
            getline(iss, dateTime);

            bool favorite = (trim(yesNo) == "yes");
            int visitCount = stoi(trim(visitCount_str));
            time_t lastVisited = parseDateTime(trim(dateTime));

            bookMarkNode* bookmark = createBookmarkNode(currentPage, favorite, visitCount, lastVisited);
            addBookmarkToList(bookmarks, bookmark);
            bookmarksCount++;
        }
    }

    inputFile.close();

    cout << "Loaded: " << pagesCount << " pages, "
        << relatedCount << " related, "
        << bookmarksCount << " bookmarks" << endl;

    return true;
}

// =================== FUNCTION 2: CREATE OR OPEN PAGE ===================

Page* createOrOpenPage(pages_History* history, Page* newPage, related_Pages* newPagerp) {

    // Check if page already exists
    Page* existing = findPageByURL(history, newPage->URL);

    if (existing != nullptr) {
        existing->visitedAt = time(nullptr);
        cout << "Opened existing page: " << existing->title << endl;
        return existing;
    }

    // Create new page
    newPage->P_ID = generateNextPageID(history);
    newPage->visitedAt = time(nullptr);
    newPage->prev = nullptr;
    newPage->next = nullptr;
    newPage->rp_head = nullptr;

    if (newPagerp != nullptr) {
        int startingRID = calculateRelatedPageStartID(newPage->P_ID);
        related_Pages* rpRunner = newPagerp;
        int count = 0;

        while (rpRunner != nullptr) {
            rpRunner->R_ID = startingRID + count;
            count++;
            rpRunner = rpRunner->next;
        }

        newPage->rp_head = newPagerp;
    }

    addPageToHistory(history, newPage);
    cout << "Created new page: " << newPage->title << " (P_ID: " << newPage->P_ID << ")" << endl;

    return newPage;
}

// ========================= BOOKMARK FUNCTIONS =========================

void updateBookmarkVisitCount(bookMarkList* bookmarks, Page* page) {
    bookMarkNode* cur = bookmarks->head;

    while (cur != nullptr) {
        if (cur->page == page) {
            cur->visitCount++;
            cur->lastVisited = time(nullptr);
            return;
        }
        cur = cur->next;
    }
}

void bookmarkRelatedPage(pages_History* history, bookMarkList* bookmarks, Page* mainPage) {
    cout << "\nRelated pages:" << endl;
    related_Pages* rp = mainPage->rp_head;
    int count = 1;

    while (rp != nullptr) {
        cout << count++ << ". " << rp->title << " (" << rp->URL << ")" << endl;
        rp = rp->next;
    }

    if (count == 1) {
        cout << "No related pages!" << endl;
        return;
    }

    string relatedURL;
    cout << "\nEnter URL of related page to bookmark: ";
    cin >> relatedURL;

    rp = mainPage->rp_head;
    while (rp != nullptr) {
        if (rp->URL == relatedURL) {
            Page* relatedAsPage = findPageByURL(history, rp->URL);

            if (relatedAsPage == nullptr) {
                relatedAsPage = createNewPage(generateNextPageID(history), rp->URL, rp->title);
                addPageToHistory(history, relatedAsPage);
            }

            if (isBookmarked(bookmarks, relatedAsPage)) {
                updateBookmarkVisitCount(bookmarks, relatedAsPage);
                cout << "Bookmark updated!" << endl;
                return;
            }

            char favChoice;
            cout << "Mark as favorite? (y/n): ";
            cin >> favChoice;

            bookMarkNode* newBookmark = createBookmarkNode(
                relatedAsPage,
                (favChoice == 'y' || favChoice == 'Y'),
                1,
                time(nullptr)
            );

            addBookmarkToList(bookmarks, newBookmark);
            cout << "Related page bookmarked!" << endl;
            return;
        }
        rp = rp->next;
    }

    cout << "URL not found!" << endl;
}

void addNewBookmarktoPage(bookMarkList* bookmarks, pages_History* history, Page* newPage) {

    if (isBookmarked(bookmarks, newPage)) {
        updateBookmarkVisitCount(bookmarks, newPage);
        cout << "Bookmark updated!" << endl;

        if (newPage->rp_head != nullptr) {
            char choice;
            cout << "\nBookmark a related page? (y/n): ";
            cin >> choice;
            if (choice == 'y' || choice == 'Y') {
                bookmarkRelatedPage(history, bookmarks, newPage);
            }
        }
        return;
    }

    char favChoice;
    cout << "Mark as favorite? (y/n): ";
    cin >> favChoice;

    bookMarkNode* newBookmark = createBookmarkNode(
        newPage,
        (favChoice == 'y' || favChoice == 'Y'),
        1,
        time(nullptr)
    );

    addBookmarkToList(bookmarks, newBookmark);
    cout << "Page bookmarked!" << endl;

    if (newPage->rp_head != nullptr) {
        char relChoice;
        cout << "\nBookmark a related page? (y/n): ";
        cin >> relChoice;
        if (relChoice == 'y' || relChoice == 'Y') {
            bookmarkRelatedPage(history, bookmarks, newPage);
        }
    }
}

void bookmarkPageByID(pages_History* history, bookMarkList* bookmarks) {
    int P_ID;
    cout << "\nEnter Page ID to bookmark: ";
    cin >> P_ID;

    Page* current = history->head;
    while (current != nullptr) {
        if (current->P_ID == P_ID) {
            addNewBookmarktoPage(bookmarks, history, current);
            return;
        }
        current = current->next;
    }

    cout << "Page not found!" << endl;
}

// =================== FUNCTION 3: REMOVE OLD PAGES ===================

void removeBookmarksForPage(bookMarkList* bookmarks, Page* page) {
    bookMarkNode* curr = bookmarks->head;

    while (curr != nullptr) {
        if (curr->page == page) {
            bookMarkNode* temp = curr;

            if (curr->prev == nullptr) {
                bookmarks->head = curr->next;
                if (bookmarks->head != nullptr) {
                    bookmarks->head->prev = nullptr;
                }
                else {
                    bookmarks->tail = nullptr;
                }
                curr = bookmarks->head;
            }
            else if (curr->next == nullptr) {
                bookmarks->tail = curr->prev;
                bookmarks->tail->next = nullptr;
                curr = nullptr;
            }
            else {
                curr->prev->next = curr->next;
                curr->next->prev = curr->prev;
                curr = curr->next;
            }

            delete temp;
        }
        else {
            curr = curr->next;
        }
    }
}

pages_History* removeHistWithDate(pages_History* history, bookMarkList* bookmarks, time_t date) {
    if (history->head == nullptr) {
        cout << "History is empty!" << endl;
        return history;
    }

    Page* curr = history->head;
    int removedCount = 0;

    while (curr != nullptr) {
        if (curr->visitedAt < date) {
            Page* temp = curr;


            related_Pages* runner = temp->rp_head;
            while (runner != nullptr) {
                related_Pages* next = runner->next;
                delete runner;
                runner = next;
            }


            removeBookmarksForPage(bookmarks, temp);


            if (curr->prev == nullptr) {
                history->head = curr->next;
                if (history->head != nullptr) {
                    history->head->prev = nullptr;
                }
                else {
                    history->tail = nullptr;
                }
                curr = history->head;
            }
            else if (curr->next == nullptr) {
                history->tail = curr->prev;
                history->tail->next = nullptr;
                curr = nullptr;
            }
            else {
                curr->prev->next = curr->next;
                curr->next->prev = curr->prev;
                curr = curr->next;
            }

            delete temp;
            removedCount++;
        }
        else {
            curr = curr->next;
        }
    }

    cout << "Removed " << removedCount << " pages" << endl;
    return history;
}

void removeOldPagesMenu(pages_History* history, bookMarkList* bookmarks) {
    cout << "\nEnter date (DD/MM/YYYY): ";

    int day, month, year;
    char delimiter;
    cin >> day >> delimiter >> month >> delimiter >> year;

    struct tm cutoffTime = {};
    cutoffTime.tm_mday = day;
    cutoffTime.tm_mon = month - 1;
    cutoffTime.tm_year = year - 1900;
    cutoffTime.tm_hour = 0;
    cutoffTime.tm_min = 0;
    cutoffTime.tm_sec = 0;

    time_t cutoffDate = mktime(&cutoffTime);

    // Preview
    Page* curr = history->head;
    int count = 0;
    while (curr != nullptr) {
        if (curr->visitedAt < cutoffDate) {
            count++;
        }
        curr = curr->next;
    }

    if (count == 0) {
        cout << "No pages to remove." << endl;
        return;
    }

    char confirm;
    cout << "\nRemove " << count << " pages? (y/n): ";
    cin >> confirm;

    if (confirm == 'y' || confirm == 'Y') {
        removeHistWithDate(history, bookmarks, cutoffDate);
    }
    else {
        cout << "Cancelled." << endl;
    }
}

// =================== FUNCTION 4: MOST RELATED PAGE ===================

Page* mostRelatedPage(pages_History* history) {
    if (history->head == nullptr) {
        return nullptr;
    }

    Page* curr = history->head;
    int max = 0;
    Page* maxCurr = nullptr;

    while (curr != nullptr) {
        related_Pages* rp_curr = curr->rp_head;
        int count = 0;

        while (rp_curr != nullptr) {
            count++;
            rp_curr = rp_curr->next;
        }

        if (count > max) {
            max = count;
            maxCurr = curr;
        }

        curr = curr->next;
    }

    return maxCurr;
}

void displayMostRelatedPage(pages_History* history) {
    Page* result = mostRelatedPage(history);

    if (result == nullptr) {
        cout << "No pages with related pages found!" << endl;
        return;
    }

    int count = 0;
    related_Pages* rp = result->rp_head;
    while (rp != nullptr) {
        count++;
        rp = rp->next;
    }

    cout << "\nMost related page:" << endl;
    cout << "P_ID: " << result->P_ID << endl;
    cout << "Title: " << result->title << endl;
    cout << "Related pages: " << count << endl;

    rp = result->rp_head;
    int num = 1;
    while (rp != nullptr) {
        cout << "  " << num++ << ". " << rp->title << endl;
        rp = rp->next;
    }
}

// =================== FUNCTION 5: DELETE PAGE ===================

void deleteThisPage(pages_History* history, bookMarkList* bookmarks, Page* pageToDelete) {

    // Remove bookmarks
    bookMarkNode* cur2 = bookmarks->head;
    while (cur2 != nullptr) {
        if (cur2->page == pageToDelete) {
            bookMarkNode* temp = cur2;

            if (cur2->prev == nullptr) {
                bookmarks->head = cur2->next;
                if (bookmarks->head != nullptr) {
                    bookmarks->head->prev = nullptr;
                }
                else {
                    bookmarks->tail = nullptr;
                }
                cur2 = bookmarks->head;
            }
            else if (cur2->next == nullptr) {
                bookmarks->tail = cur2->prev;
                bookmarks->tail->next = nullptr;
                cur2 = nullptr;
            }
            else {
                cur2->prev->next = cur2->next;
                cur2->next->prev = cur2->prev;
                cur2 = cur2->next;
            }
            delete temp;
        }
        else {
            cur2 = cur2->next;
        }
    }

    // Delete related pages
    related_Pages* cur3 = pageToDelete->rp_head;
    while (cur3 != nullptr) {
        related_Pages* next = cur3->next;
        delete cur3;
        cur3 = next;
    }

    // Remove from history
    if (pageToDelete->prev == nullptr) {
        history->head = pageToDelete->next;
        if (history->head != nullptr) {
            history->head->prev = nullptr;
        }
        else {
            history->tail = nullptr;
        }
    }
    else if (pageToDelete->next == nullptr) {
        history->tail = pageToDelete->prev;
        history->tail->next = nullptr;
    }
    else {
        pageToDelete->prev->next = pageToDelete->next;
        pageToDelete->next->prev = pageToDelete->prev;
    }

    delete pageToDelete;
}

void deletePageMenu(pages_History* history, bookMarkList* bookmarks) {
    cout << "\nEnter Page ID to delete: ";

    int P_ID;
    cin >> P_ID;

    Page* curr = history->head;
    while (curr != nullptr) {
        if (curr->P_ID == P_ID) {
            cout << "Page: " << curr->title << endl;

            char confirm;
            cout << "Delete this page? (y/n): ";
            cin >> confirm;

            if (confirm == 'y' || confirm == 'Y') {
                deleteThisPage(history, bookmarks, curr);
                cout << "Page deleted!" << endl;
            }
            else {
                cout << "Cancelled." << endl;
            }
            return;
        }
        curr = curr->next;
    }

    cout << "Page not found!" << endl;
}

// =================== FUNCTION 6: GET RECENT BOOKMARKS ===================

bookMarkNode** getRecentBookmarks(bookMarkList* bookmarks, int N) {

    if (bookmarks->head == nullptr || N <= 0) {
        return nullptr;
    }

    int totalCount = 0;
    bookMarkNode* counter = bookmarks->head;
    while (counter != nullptr) {
        totalCount++;
        counter = counter->next;
    }

    if (N > totalCount) {
        N = totalCount;
    }

    bookMarkNode** result = new bookMarkNode * [N];
    for (int i = 0; i < N; i++) {
        result[i] = nullptr;
    }

    bookMarkNode* curr = bookmarks->head;

    while (curr != nullptr) {
        int insertPos = -1;

        for (int i = 0; i < N; i++) {
            if (result[i] == nullptr) {
                insertPos = i;
                break;
            }
            else if (curr->lastVisited > result[i]->lastVisited) {
                insertPos = i;
                break;
            }
        }

        if (insertPos != -1) {
            for (int i = N - 1; i > insertPos; i--) {
                result[i] = result[i - 1];
            }
            result[insertPos] = curr;
        }

        curr = curr->next;
    }

    return result;
}

void displayRecentBookmarks(bookMarkList* bookmarks) {
    cout << "\nHow many recent bookmarks? ";

    int N;
    cin >> N;

    if (N <= 0) {
        cout << "Invalid number!" << endl;
        return;
    }

    bookMarkNode** recent = getRecentBookmarks(bookmarks, N);

    if (recent == nullptr) {
        cout << "No bookmarks found!" << endl;
        return;
    }

    int resultCount = 0;
    for (int i = 0; i < N; i++) {
        if (recent[i] != nullptr) {
            resultCount++;
        }
        else {
            break;
        }
    }

    cout << "\n" << resultCount << " most recent bookmarks:" << endl;

    for (int i = 0; i < resultCount; i++) {
        bookMarkNode* bm = recent[i];
        cout << "\n[" << (i + 1) << "] " << bm->page->title << endl;
        cout << "    Visits: " << bm->visitCount << endl;
        cout << "    Last visited: " << formatDateTime(bm->lastVisited) << endl;
    }

    delete[] recent;
}

// =================== FUNCTION 7: SEARCH BY URL ===================

void searchByURL(pages_History* history, const string& substring) {

    if (history->head == nullptr) {
        cout << "No pages in history!" << endl;
        return;
    }

    if (substring.empty()) {
        cout << "Substring cannot be empty!" << endl;
        return;
    }

    string lowerSubstring = substring;
    for (size_t i = 0; i < lowerSubstring.length(); i++) {
        lowerSubstring[i] = tolower(lowerSubstring[i]);
    }

    int foundCount = 0;
    Page* curr = history->head;

    while (curr != nullptr) {
        string lowerURL = curr->URL;
        for (size_t i = 0; i < lowerURL.length(); i++) {
            lowerURL[i] = tolower(lowerURL[i]);
        }

        if (lowerURL.find(lowerSubstring) != string::npos) {
            foundCount++;
            cout << "\n[" << foundCount << "] " << curr->title << endl;
            cout << "    URL: " << curr->URL << endl;
        }

        curr = curr->next;
    }

    if (foundCount == 0) {
        cout << "\nNo pages found!" << endl;
    }
    else {
        cout << "\nFound " << foundCount << " page(s)" << endl;
    }
}

void searchByURLMenu(pages_History* history) {
    cout << "\nEnter URL substring: ";

    cin.ignore();
    string substring;
    getline(cin, substring);

    searchByURL(history, substring);
}

// =================== FUNCTION 8: REMOVE LEAST VISITED BOOKMARK ===================

void removeLeastVisitedBookmark(bookMarkList* bookmarks) {

    if (bookmarks->head == nullptr) {
        cout << "No bookmarks to remove!" << endl;
        return;
    }

    if (bookmarks->head == bookmarks->tail) {
        delete bookmarks->head;
        bookmarks->head = nullptr;
        bookmarks->tail = nullptr;
        cout << "Removed only bookmark!" << endl;
        return;
    }

    bookMarkNode* curr = bookmarks->head;
    bookMarkNode* minNode = curr;
    int min = curr->visitCount;

    while (curr != nullptr) {
        if (curr->visitCount < min) {
            min = curr->visitCount;
            minNode = curr;
        }
        curr = curr->next;
    }

    if (minNode->prev == nullptr) {
        bookmarks->head = minNode->next;
        if (bookmarks->head != nullptr) {
            bookmarks->head->prev = nullptr;
        }
        else {
            bookmarks->tail = nullptr;
        }
    }
    else if (minNode->next == nullptr) {
        bookmarks->tail = minNode->prev;
        bookmarks->tail->next = nullptr;
    }
    else {
        minNode->prev->next = minNode->next;
        minNode->next->prev = minNode->prev;
    }

    delete minNode;
    cout << "Removed least visited bookmark!" << endl;
}

void removeLeastVisitedBookmarkMenu(bookMarkList* bookmarks) {

    if (bookmarks->head == nullptr) {
        cout << "No bookmarks!" << endl;
        return;
    }

    bookMarkNode* curr = bookmarks->head;
    bookMarkNode* minNode = curr;
    int min = curr->visitCount;

    while (curr != nullptr) {
        if (curr->visitCount < min) {
            min = curr->visitCount;
            minNode = curr;
        }
        curr = curr->next;
    }

    cout << "\nLeast visited: " << minNode->page->title << " (" << minNode->visitCount << " visits)" << endl;

    char confirm;
    cout << "Delete this bookmark? (y/n): ";
    cin >> confirm;

    if (confirm == 'y' || confirm == 'Y') {
        removeLeastVisitedBookmark(bookmarks);
    }
    else {
        cout << "Cancelled." << endl;
    }
}

void swapRelatedpages(pages_History* history, int P_ID1, int P_ID2) {
    if (!history->head)
        return;
    
    Page* curr = history->head;
    Page* curr2;
    while (curr) {
        curr2 = curr;
        while (curr2) {
            if (curr->P_ID == P_ID1 && curr2->P_ID == P_ID2) {
                related_Pages* runner1 = curr->rp_head;
                related_Pages* runner2 = curr2->rp_head;
                while (runner1 && runner2) {
                    int x = 0;
                    x = runner1->R_ID;
                    runner1->R_ID = runner2->R_ID;
                    runner2->R_ID = x;
                    string y;
                    y = runner1->URL;
                    runner1->URL = runner2->URL;
                    runner2->URL = y;
                    string t;
                    t = runner1->URL;
                    runner1->URL = runner2->URL;
                    runner2->URL = t;
                    cout << "related pages are swaped";

                    runner1 = runner1->next;
                    runner2 = runner2->next;
                }
                if (!runner1 && runner2) {
                    addRelatedToPage(curr, runner2);
                }
                else {
                    if (!runner2 && runner1) {
                        addRelatedToPage(curr2, runner1);

                    }
                }


            }
            else curr2 = curr->next;
        }
        curr = curr->next;
        
    }

    cout << "done";
    }

bool findpagebyID(pages_History* history, int id1) {
    Page* curr = history->head;
    while (curr) {
        if (curr->P_ID == id1) {
            return true;
        }
        curr = curr->next;
    }
    cout << "ID is not valid";
    return false;
    }

void Getoption(pages_History* history) {

    int x1, x2;
    cout << "give me two ids of pages in order to swap their related pages" << endl;
    cin >> x1;
    findpagebyID(history, x1);
    cout << "id not found";
    while (!findpagebyID(history, x1)) {
        cin >> x1;
    }

    cin >> x2;
    findpagebyID(history, x2);
    cout << "id not found";
    while (!findpagebyID(history, x2)) {
        cin >> x2;
    }

    swapRelatedpages(history, x1, x2);
    return;

}

void RemoveMostVisited(bookMarkList* bookmarks) {
    bookMarkNode* curr = bookmarks->head;
    bookMarkNode* maxNode = NULL;
    int max = 0;
    while (curr) {

        if (curr->visitCount > max) {
            max = curr-> visitCount;
            maxNode = curr;
        }

        curr = curr->next;


   }

    if (maxNode->prev == nullptr) {
        bookmarks->head = maxNode->next;
        if (bookmarks->head != nullptr) {
            bookmarks->head->prev = nullptr;
        }
        else {
            bookmarks->tail = nullptr;
        }
    }
    else if (maxNode->next == nullptr) {
        bookmarks->tail = maxNode->prev;
        bookmarks->tail->next = nullptr;
    }
    else {
        maxNode->prev->next = maxNode->next;
        maxNode->next->prev = maxNode->prev;
    }

    delete maxNode;
    cout << "Removed least visited bookmark!" << endl;



}


related_Pages* initialise1() {
    related_Pages* newlist_head = nullptr;
    return newlist_head;
}

related_Pages* initialise() {
    related_Pages* newlist_tail = nullptr;
    return newlist_tail;
}

related_Pages* deleteRelatedPage(pages_History* history, int Rid,related_Pages* newlist_head,related_Pages* newlist_tail) {

    Page* curr = history->head;
    while (curr) {
        related_Pages* runner = curr->rp_head;
        related_Pages* prev = nullptr;
        while (runner) {
            if (runner->R_ID == Rid) {
                if (prev == nullptr) {
                    curr->rp_head = curr->rp_head->next;
                    runner->next = nullptr;
                    newlist_head = newlist_tail = runner;
                    
                    }

                else {
                    prev->next = runner->next;
                    runner->next = nullptr;
                    if (!newlist_tail) {
                        newlist_head = newlist_tail = runner;
                    }
                    else {
                        newlist_tail->next = runner;
                        newlist_tail = runner;
                    }
                }


            }
        }
    }



    return newlist_head;

}


void displayDeletedRelatedPages(related_Pages* newlist_head) {
    related_Pages* curr = newlist_head;
    while (curr) {
        cout << curr->R_ID << endl;
        cout << curr->URL << endl;
        cout << curr->title << endl;
        curr = curr->next;
    }
}

void menuTodeleteRelatedPage(pages_History* history, related_Pages* newlist_head, related_Pages* newlist_tail) {
    int id;
    cout << "enter related Page ID to delete" << endl;
    cin>>id;


}














// =================== FUNCTION 9: SAVE TO FILE ===================

bool saveToFile(const string& filename, pages_History* history, bookMarkList* bookmarks) {

    ofstream outFile(filename.c_str());

    if (!outFile.is_open()) {
        cout << "Could not open file for writing!" << endl;
        return false;
    }

    Page* currPage = history->head;

    while (currPage != nullptr) {

        outFile << "-" << currPage->P_ID << ", "
            << currPage->URL << ", "
            << currPage->title << ", "
            << formatDateTime(currPage->visitedAt) << endl;

        related_Pages* currRelated = currPage->rp_head;
        while (currRelated != nullptr) {
            outFile << "*" << currRelated->R_ID << ", "
                << currRelated->URL << ", "
                << currRelated->title << endl;
            currRelated = currRelated->next;
        }

        bookMarkNode* currBookmark = bookmarks->head;
        while (currBookmark != nullptr) {
            if (currBookmark->page == currPage) {
                string favoriteStr = currBookmark->favorite ? "yes" : "no";
                outFile << "#" << favoriteStr << ", "
                    << currBookmark->visitCount << ", "
                    << formatDateTime(currBookmark->lastVisited) << endl;
                break;
            }
            currBookmark = currBookmark->next;
        }

        currPage = currPage->next;
    }

    outFile.close();
    cout << "Saved successfully!" << endl;
    return true;
}

void saveToFileMenu(pages_History* history, bookMarkList* bookmarks) {
    cout << "\nEnter filename: ";

    cin.ignore();
    string filename;
    getline(cin, filename);

    if (filename.empty()) {
        cout << "Filename cannot be empty!" << endl;
        return;
    }

    saveToFile(filename, history, bookmarks);
}
















// ========================= DISPLAY FUNCTIONS =========================

void displayAllPages(pages_History* history) {
    cout << "\n=== ALL PAGES ===" << endl;

    if (history->head == nullptr) {
        cout << "No pages." << endl;
        return;
    }

    Page* current = history->head;
    int pageNum = 1;

    while (current != nullptr) {
        cout << "\n[" << pageNum++ << "] P_ID " << current->P_ID << ": " << current->title << endl;
        cout << "    " << current->URL << endl;
        current = current->next;
    }
}

void displayBookmarks(bookMarkList* bookmarks) {
    cout << "\n=== BOOKMARKS ===" << endl;

    if (bookmarks->head == nullptr) {
        cout << "No bookmarks." << endl;
        return;
    }

    bookMarkNode* current = bookmarks->head;
    int count = 1;

    while (current != nullptr) {
        cout << "\n[" << count++ << "] " << current->page->title << endl;
        cout << "    Visits: " << current->visitCount << endl;
        current = current->next;
    }
}

// ========================= USER INPUT =========================

void getUserPagesAndCreate(pages_History* history, bookMarkList* bookmarks) {

    cout << "\n=== MAIN PAGE ===" << endl;
    string mainURL = getValidURL();
    string mainTitle = getValidTitle();

    Page* newPage = new Page;
    newPage->URL = mainURL;
    newPage->title = mainTitle;

    cout << "\nHow many related pages? ";
    int numRelated;
    cin >> numRelated;

    if (numRelated < 0) numRelated = 0;

    related_Pages* rpHead = nullptr;
    related_Pages* rpTail = nullptr;

    for (int i = 0; i < numRelated; i++) {
        cout << "\n--- Related Page " << (i + 1) << " ---" << endl;
        string rpURL = getValidURL();
        string rpTitle = getValidTitle();

        related_Pages* newRP = createRelatedPage(0, rpURL, rpTitle);

        if (rpHead == nullptr) {
            rpHead = rpTail = newRP;
        }
        else {
            rpTail->next = newRP;
            rpTail = newRP;
        }
    }

    Page* result = createOrOpenPage(history, newPage, rpHead);
    updateBookmarkVisitCount(bookmarks, result);
}

// ========================= MAIN =========================

int main() {
    cout << "=== BROWSER HISTORY MANAGER ===" << endl;

    pages_History history;
    bookMarkList bookmarks;

    initializeHistory(&history);
    initializeBookmarkList(&bookmarks);

    parseInputFile("input.txt", &history, &bookmarks);

    while (true) {
        cout << "\n=== MENU ===" << endl;
        cout << "1. Create/Open page" << endl;
        cout << "2. Add bookmark" << endl;
        cout << "3. View all pages" << endl;
        cout << "4. View bookmarks" << endl;
        cout << "5. Remove old pages" << endl;
        cout << "6. Find most related page" << endl;
        cout << "7. Delete a page" << endl;
        cout << "8. Get recent bookmarks" << endl;
        cout << "9. Search by URL" << endl;
        cout << "10. Remove least visited bookmark" << endl;
        cout << "11. Save to file" << endl;
        cout << "12. Exit" << endl;
        cout << "Choice: ";

        int choice;
        cin >> choice;


        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "\n✗ Invalid input! Please enter a number (1-12)." << endl;
            continue;
        }


        switch (choice) {
        case 1:
            getUserPagesAndCreate(&history, &bookmarks);
            break;
        case 2:
            if (history.head == nullptr) {
                cout << "No pages in history!" << endl;
            }
            else {
                displayAllPages(&history);
                bookmarkPageByID(&history, &bookmarks);
            }
            break;
        case 3:
            displayAllPages(&history);
            break;
        case 4:
            displayBookmarks(&bookmarks);
            break;
        case 5:
            if (history.head == nullptr) {
                cout << "No pages in history!" << endl;
            }
            else {
                removeOldPagesMenu(&history, &bookmarks);
            }
            break;
        case 6:
            if (history.head == nullptr) {
                cout << "No pages in history!" << endl;
            }
            else {
                displayMostRelatedPage(&history);
            }
            break;
        case 7:
            if (history.head == nullptr) {
                cout << "No pages in history!" << endl;
            }
            else {
                deletePageMenu(&history, &bookmarks);
            }
            break;
        case 8:
            if (bookmarks.head == nullptr) {
                cout << "No bookmarks!" << endl;
            }
            else {
                displayRecentBookmarks(&bookmarks);
            }
            break;
        case 9:
            if (history.head == nullptr) {
                cout << "No pages in history!" << endl;
            }
            else {
                searchByURLMenu(&history);
            }
            break;
        case 10:
            if (bookmarks.head == nullptr) {
                cout << "No bookmarks!" << endl;
            }
            else {
                removeLeastVisitedBookmarkMenu(&bookmarks);
            }
            break;
        case 11:
            saveToFileMenu(&history, &bookmarks);
            break;
        case 12:
            cout << "Goodbye!" << endl;
            return 0;
        default:
            cout << "Invalid option!" << endl;
        }
    }

    return 0;
}
