資料庫hw05
姓名: 龔鈺閎
系級: 資工四
學號: 408410046

使用語言: C
編譯器版本: gcc 9.4.0
執行環境: WSL2

如何操作我的程式:
    1.  makefile中有寫好兩組測資，分別為hw05.pdf中的兩個範例測資。可以直接在command line中輸入"make test1"
        與"make test2"即可。
    2.  使用command line以及arguments來執行。在command line中輸入 
        ./BTree order data1 data2 data3 .......以此類推，資料個數可自行追加。
    3.  要清空所有執行檔，在command line中輸入"make clean"即可。

設計邏輯:
    1.  參考投影片以及資料結構聖經"基礎資料結構使用C"內的B+ Tree結構。
    2.  參數中包含了B+ Tree的order，我的B+ Tree是根據投影片的要求定義的struct bplus_tree， 
        包含了keys、children、value、is_leaf、key_cnt、prev、next這七個變數。
        B+ Tree的節點可以分為兩種，一為葉節點，另一種為非葉節點。B+ Tree的所有葉節點為一個linked list，是
        透過struct bplus_tree中的prev與next兩個pointer來實作double linked list，並且將is_leaf設為true。
        非葉節點的is_leaf為false，內部會存需要搜尋的key以及對應的children的指標，以及統計該節點中key數量的
        變數key_cnt。

        輸入的資料是存放在argv中，透過argc去判斷有多少個參數。這份作業主要的function為:
            a. initialize()
            b. insert()
            c. Display()
            d. quit()
        
        a.  initialize()功能為初始化新的Node，傳入的參數有value、order、isLeaf。若isLeaf為true時，則代表
            該node為leaf node，並且要insret的value是assign給val。若isLeaf為false，則該節點為internal node
            ，要insert的value要assign給key。order主要是決定要分配多少空間給key以及children。若order為x，
            則key就有2 * x個、children有2 * x + 1個、val有2 * x個。

        b.  insert()功能為插入data，並且平衡B+ Tree。傳入的參數有root、data、order。root為B+ Tree的根節點，
            是透過root去找到data要insert的leaf node。找到leaf node之後會先判斷該node有沒有空間，有的化就直接
            插入到該leaf node並回傳root。若沒有空間的話，則需要建立新的leaf node來平衡，建立新的leaf node之後
            會先將該node最小值插入回去他的parent node，一路往上調整，最後回傳根結點。向上調整的功能使用遞迴的方
            式去呈現。

        c.  Display()功能是將B+ Tree以post order的順序印出來。這個function是用遞迴的方式實作。傳入的參數有
            root、order、tab_cnt。透過B+ Tree的root可以找到所有節點，order則是知道最多有幾個children，tab_cnt
            則是控制最後輸出，不同層的key會利用tab的數量去對齊排版。

        d.  quit()功能是free B+ Tree。透過post order的方式去釋放記憶體。傳入的參數有root跟order，透過root
            開始往下找節點，order則是控制每個node有多少個children。

    3.  輸入為command line的argument，輸出的部分，對於每一次iteration，都會先印出該iteration要輸入的value，並且
        在插入到B+ Tree之後印出Tree一次。