#ifndef VRAPP_H
#define VRAPP_H

#include <vector>
#include <string>

#define MENU_OPTIONS_MAIN_SIZE    6
#define MENU_OPTIONS_ADD_SIZE    10
#define MENU_OPTIONS_EDIT_SIZE    2
#define MENU_OPTIONS_TRANS_SIZE  12
#define MENU_SUBCHOICES           4
#define MENU_ANGLES_PER_CHOICE   15
enum MenuOptions1 {
    MENU_ADD,
    MENU_EDIT,
    MENU_DELETE,
    MENU_MOVE,
    MENU_ROTATE,
    MENU_SCALE,
};
enum MenuOptions2 {
    MENU_TRANSFORM_X,
    MENU_TRANSFORM_Y,
    MENU_TRANSFORM_Z,
    MENU_TRANSFORM_XYZ,
};
enum MenuOptions3 {
    MENU_EDI_TEXTURE,
    MENU_EDIT_CLONE,
};
/////////////////////////////////////////////////////
typedef struct{
    std::string name;
    std::vector<int> models;
}Category;


#define VR_M_RAY     0
#define VR_M_MENU    0
#define VR_M_LOADING 1
#define VR_M_FORWARD 2

class VrApp
{
    public:
        VrApp();
        void loadCategorizedModels();
        std::string getFileName(int category, int id, int mat);
        std::vector<Category> categories={
            {"Chair"},
            {"Sofa"},
            {"Table"},
            {"Carpet"},
            {"Desk"},
            {"Shelf"},
            {"Bed"},
            {"Wardrobe"},
            {"Other"},
            {"Room"},
        };
        std::string imPath="Models/VR_icons/";
        std::string imMain [MENU_OPTIONS_MAIN_SIZE ]={
                // Main Menu Options (6)
                "Add.png",
                "Edit.png",
                "Delete.png",
                "Move.png",
                "Rotate.png",
                "Scale.png",
        };
        std::string imAdd  [MENU_OPTIONS_ADD_SIZE  ]={
                // Add Menu Options (8 Categories)
                "Add_chair.png",
                "Add_sofa.png",
                "Add_table.png",
                "Add_carpet.png",
                "Add_desk.png",
                "Add_shelf.png",
                "Add_bed.png",
                "Add_wardrobe.png",
                "Add_others.png",
                "Add_room.png",
        };
        std::string imEdit[MENU_OPTIONS_EDIT_SIZE]={
                // Edit Menu Options (2)
                "Edit_texture.png",
                "Edit_clone.png",
        };
        std::string imTrans[MENU_OPTIONS_TRANS_SIZE]={
                // Transform Menu Options (12)
                // Move [4]
                "Move_X.png",
                "Move_Y.png",
                "Move_Z.png",
                "Move_XYZ.png",
                // Rotate [4]
                "Rotate_X.png",
                "Rotate_Y.png",
                "Rotate_Z.png",
                "Rotate_XYZ.png",
                // Scale [4]
                "Scale_X.png",
                "Scale_Y.png",
                "Scale_Z.png",
                "Scale_XYZ.png",
        };

        // Transparency mask for model images
        std::string imModelMask="model_mask.png";

        int level=0;
        int choice[5];
        float cursor=0;

    protected:

    private:
};

#endif // VRAPP_H
