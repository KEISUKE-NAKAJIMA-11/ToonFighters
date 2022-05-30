#include "obj2d.h"



int OBJ2D::damage = 0;
OBJ2D::OBJ2D()
{
    clear();

    scale = VECTOR2(1, 1);          // 初期化忘れが無いように（スケールゼロなら何も表示されない）
    color = VECTOR4(1, 1, 1, 1);    // 初期化忘れが無いように（不透明度ゼロなら何も表示されない）
}

//--------------------------------
//  メンバ変数のクリア
//--------------------------------
void OBJ2D::clear()
{
    data = nullptr;
    SecureZeroMemory(&anime, sizeof(anime));

    position = VECTOR2(0, 0);
    scale = VECTOR2(0, 0);
    angle = 0.0f;
    color = VECTOR4(0, 0, 0, 0);
    size = VECTOR2(0, 0);

    mvAlg = nullptr;
    eraseAlg = nullptr;

    velocity = 0.0f;
    speed = VECTOR2(0, 0);
    state = 0;
    timer = 0;
    damageCount = 0;

    score = 0;
    param = 0;
    jumpTimer = 0;

    SecureZeroMemory(iWork, sizeof(iWork));
    SecureZeroMemory(fWork, sizeof(fWork));

    hashigoFlag = false;
    drawHitRectFlag = false;
    judgeFlag = false;
    pad[0] = 0;
}


void OBJ2D::move()
{
    if (mvAlg) mvAlg->move(this);                   // 移動アルゴリズムが存在すれば、moveメソッドを呼ぶ
    if (eraseAlg) eraseAlg->erase(this);            // 消去アルゴリズムが存在すれば、eraseメソッドを呼ぶ
}

//--------------------------------
//  描画
//--------------------------------
void OBJ2D::draw()
{
    if (data)                                       // OBJ2Dのdataメンバにスプライトデータがあれば
    {
        data->draw(position, scale, angle, color);  // dataのdrawメソッドでスプライトを描画する
    }
}

void OBJ2D::hoseirect()
{
    hitRect.right = position.x + size.x;
    hitRect.left = position.x - size.x;
    hitRect.top = position.y - size.y;
    hitRect.bottom = position.y;

}


bool OBJ2D::animeUpdate()
{
    if (animeData == nullptr) assert(!"animeUpdate関数でAnimeDataがnullptr");

    if (anime.pPrev != animeData)           // アニメデータが切り替わったとき
    {
        anime.pPrev = animeData;
        anime.patNum = 0;	                // 先頭から再生
        anime.frame = 0;
    }

    animeData += anime.patNum;
    data = animeData->data;                 // 現在のパターン番号に該当する画像を設定

    anime.frame++;
    if (anime.frame >= animeData->frame)    // 設定フレーム数表示したら
    {
        anime.frame = 0;
        anime.patNum++;                     // 次のパターンへ
        if ((animeData + 1)->frame < 0)     // 終了コードのとき
        {
            anime.patNum = 0;               // 先頭へ戻る
            return true;
        }
    }

    return false;
}




void OBJ2DManager::init()
{

    objList.clear();
}


OBJ2D* OBJ2DManager::add(MoveAlg2D* mvAlg, const VECTOR2& pos)
{
    OBJ2D obj;                          // OBJ2Dを宣言する
    obj.mvAlg = mvAlg;                  // mvAlgに引数のmvalgを代入
    obj.position = pos;                 // positionに引数のposを代入
    obj.scale = VECTOR2(1, 1);          // スケールは等倍
    obj.color = VECTOR4(1, 1, 1, 1);    // 色は白（不透明度1.0f）

    objList.push_back(obj);             // リストにobjを追加する
    return &(*objList.rbegin());        // 今追加したobjのアドレスを返す（何かで使えるように）
}


void OBJ2DManager::update()
{
    // 移動
    for (auto& it : objList)            // objListの全ての要素をループし、itという名前で各要素にアクセス
    {
        it.move();                      // itのmoveメソッドを呼ぶ
    }

    // 空ノードの削除（今は気にしなくて良い）
    auto it = objList.begin();
    while (it != objList.end())
    {
        if (it->mvAlg)
        {
            it++;
        }
        else
        {
            it = objList.erase(it);
        }
    }
}



void OBJ2DManager::draw()
{
    for (auto& it : objList)            // objListの全ての要素をループし、itという名前で各要素にアクセス 
    {
        it.draw();                      // itのdrawメソッドを呼ぶ
    }
}


OBJ2DManager::~OBJ2DManager()
{
    //リストのOBJ2Dを全てクリアする
    objList.clear();
}
