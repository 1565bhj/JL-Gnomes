
#include "stdio.h"     // 标准输入输出函数（如printf）
#include "string.h"    // 字符串操作（如strcpy复制图标名称）
#include "gui_guider.h"// LVGL界面生成器（GUI Guider）的自动生成头文件，包含界面对象（如guider_ui）
#include "events_init.h"// 事件初始化相关（虽未直接调用，但为LVGL事件机制提供支持）
#include "widgets_init.h"// 组件初始化相关（确保LVGL组件正常创建）
#include "custom.h" // 自定义功能头文件（存放项目私有逻辑）
#include <stdio.h> // 用于printf函数


#define OPTION_ICON_MOVE            50

// 图标数量
#define OPTION_ICON_COUNT          5
// 图标之间的距离
#define OPTION_ICON_DISTANCE       110
// 小图标的尺寸缩放值
#define OPTION_ICON_ZOOM_SMALL     160
// 大图标的尺寸缩放值
#define OPTION_ICON_ZOOM_BIG       240
// 判断为最大图标的阈值
#define OPTION_MAX_ICON_THRESHOLD  170

// 定义图标结构体，包含图标对象指针和x坐标
typedef struct {
    lv_obj_t *obj;
    int32_t x;
    char name_lab[20];
    bool is_max;  // 标记是否为当前最大图标
    int32_t icon_idx; // 图标索引，用于标识不同的功能
} option_icon_typedef;

// 声明图标结构体数组
static option_icon_typedef icon[OPTION_ICON_COUNT];
// 触摸状态标志
static bool touched = false;
// 屏幕宽度
static int32_t scr_w;
// 图标标签
static lv_obj_t *icon_name_lab;
// 触摸偏移量x
static int32_t t_offset_x;
// 屏幕对象指针
static lv_obj_t *screen;
// 新增：当前最大的图标索引
static int32_t current_max_icon_idx = 0;

// 按下事件回调函数声明
static void  pressing_cb(lv_event_t * e);
// 释放事件回调函数声明
static void  released_cb(lv_event_t * e);
// 设置x坐标回调函数声明
static void set_x_cb(void * var, int32_t v);
// 自定义动画创建函数声明
static void lv_myanim_creat(void * var, lv_anim_exec_xcb_t exec_cb, uint32_t time, uint32_t delay, lv_anim_path_cb_t path_cb,
                            int32_t start, int32_t end, lv_anim_ready_cb_t completed_cb, lv_anim_deleted_cb_t deleted_cb);
// 新增：图标点击事件回调函数声明
static void icon_click_cb(lv_event_t * e);
// 新增：更新最大图标状态函数声明
static void update_max_icon_status(void);
// 新增：打开对应屏幕函数声明
static void open_icon_screen(int32_t icon_idx);
void lv_open_icon_screen_set(short x, short y);
// 图标定义宏 - 修复：使用正确的图标资源名称
#define ICON_DEFINITIONS \
    ICON_DEF(0, _timer_option_alpha_100x100, "倒计时") \
    ICON_DEF(1, _alarm_option_alpha_100x100, "闹钟") \
    ICON_DEF(2, _music_option_alpha_100x100, "音乐") \
    ICON_DEF(3, _conversation_option_alpha_100x100, "对话") \
    ICON_DEF(4, _op_set_alpha_100x100, "设置")



// 滚动图标功能函数
void scrollicon(void)
{
    int32_t i;
    // 获取默认显示器的垂直分辨率
    scr_w = lv_disp_get_ver_res(lv_disp_get_default());
    // 初始化图标结构体数组为0
    lv_memset(icon, 0, sizeof(icon));

    // 创建一个平铺视图作为屏幕
    screen = lv_tileview_create(lv_scr_act());
    lv_obj_set_pos(screen, 0, 30);
    lv_obj_set_size(screen, 320, 180);

    // 清除屏幕的可滚动标志
    // 添加这行代码完全禁止页面滚动
    lv_obj_set_scroll_dir(guider_ui.screen_option, LV_DIR_NONE);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    // 将平铺视图变成透明的
    lv_obj_set_style_bg_opa(screen, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 为屏幕添加按下事件回调函数
    lv_obj_add_event_cb(screen, pressing_cb, LV_EVENT_PRESSING, 0);
    // 为屏幕添加释放事件回调函数
    lv_obj_add_event_cb(screen, released_cb, LV_EVENT_RELEASED, 0);


    // 创建图标标签
    icon_name_lab = guider_ui.screen_option_op_name;
    extern const lv_font_t lv_font_MiSansDemibold_18;
    lv_obj_set_style_text_font(icon_name_lab, &lv_font_MiSansDemibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);

    for (i = 0; i < OPTION_ICON_COUNT; i++) {
        // 创建图标对象并添加到屏幕上
        icon[i].obj = lv_img_create(screen);
        // 设置图标对象的用户数据为对应的图标结构体指针
        icon[i].obj->user_data = &icon[i];
        // 初始化是否为最大图标标志
        icon[i].is_max = false;
        // 设置图标索引
        icon[i].icon_idx = i;


        // 使用宏定义设置图标源和名称
#define ICON_DEF(idx, src, name) \
            if (i == idx) { \
                lv_img_set_src(icon[idx].obj, &src); \
                strcpy(icon[idx].name_lab, name); \
            }
        ICON_DEFINITIONS
#undef ICON_DEF
        // 计算图标x坐标
        icon[i].x = (i - OPTION_ICON_COUNT / 2) * OPTION_ICON_DISTANCE;
        // 将图标居中显示
        lv_obj_center(icon[i].obj);
        // 如果是中间的图标，设置为大尺寸
        if (i == OPTION_ICON_COUNT / 2) {
            lv_img_set_zoom(icon[i].obj, OPTION_ICON_ZOOM_BIG);
            lv_label_set_text(icon_name_lab, icon[i].name_lab);
            icon[i].is_max = true;
            current_max_icon_idx = i;
        } else {
            // 否则设置为小尺寸
            lv_img_set_zoom(icon[i].obj, OPTION_ICON_ZOOM_SMALL);

            // 修改后
            // 移除小图标的可点击标志和事件回调
            lv_obj_clear_flag(icon[i].obj, LV_OBJ_FLAG_CLICKABLE);
        }

        // 设置图标x坐标
        lv_obj_set_x(icon[i].obj, icon[i].x);
    }
}


// 按下事件回调函数
static void  pressing_cb(lv_event_t * e)
{
    static  lv_point_t click_point1, click_point2;
    int32_t v, i;

    // 如果当前未处于触摸状态
    if (touched == false) {
        for (i = 0; i < OPTION_ICON_COUNT; i++) {
            // 删除图标对象上的动画（如果有）
            lv_anim_del(icon[i].obj, set_x_cb);
        }

        // 获取当前输入设备的点击点坐标
        lv_indev_get_point(lv_indev_get_act(), &click_point1);
        // 设置触摸状态为已触摸
        touched = true;
        return;
    } else {
        // 如果已经处于触摸状态，获取当前点击点坐标
        lv_indev_get_point(lv_indev_get_act(), &click_point2);
    }

    // 计算触摸偏移量x
    t_offset_x = click_point2.x - click_point1.x;
    // 更新上一次点击点坐标
    click_point1.x = click_point2.x;

    for (int32_t i = 0; i < OPTION_ICON_COUNT; i++) {
        // 更新图标x坐标
        icon[i].x += t_offset_x;
        // 处理图标x坐标超出范围的情况（循环滚动）
        while (icon[i].x < (-OPTION_ICON_COUNT / 2) * OPTION_ICON_DISTANCE) {
            icon[i].x += (OPTION_ICON_COUNT) * OPTION_ICON_DISTANCE;
        }
        while (icon[i].x >  (OPTION_ICON_COUNT / 2) * OPTION_ICON_DISTANCE) {
            icon[i].x -= (OPTION_ICON_COUNT) * OPTION_ICON_DISTANCE;
        }
        // 设置图标对象的x坐标
        lv_obj_set_x(icon[i].obj, icon[i].x);

        // 如果图标x坐标超出一定范围，设置为小尺寸
        if (icon[i].x >= OPTION_ICON_DISTANCE || icon[i].x <= -OPTION_ICON_DISTANCE) {
            lv_img_set_zoom(icon[i].obj, OPTION_ICON_ZOOM_SMALL);
            icon[i].is_max = false;
            lv_obj_set_style_shadow_width(icon[i].obj, 0, LV_PART_MAIN);
            continue;
        }

        // 根据x坐标计算图标尺寸
        if (icon[i].x >= 0) {
            v = icon[i].x;
        } else {
            v = -icon[i].x;
        }

        // 更新图标名称标签
        if (icon[i].x >= -OPTION_ICON_MOVE && icon[i].x <= OPTION_ICON_MOVE) { // 只在中间区域更新名称
            lv_label_set_text(icon_name_lab, icon[i].name_lab);
        }

        // 计算缩放值
        int32_t zoom = OPTION_ICON_ZOOM_SMALL + (float)(OPTION_ICON_DISTANCE - v) / OPTION_ICON_DISTANCE * (OPTION_ICON_ZOOM_BIG - OPTION_ICON_ZOOM_SMALL);
        lv_img_set_zoom(icon[i].obj, zoom);

        // 更新最大图标状态
        icon[i].is_max = (zoom >= OPTION_MAX_ICON_THRESHOLD);
        // 如果是最大图标，设置阴影宽度为10

    }

    // 更新当前最大图标索引
    update_max_icon_status();
}

void lv_open_icon_screen_set(short x, short y)
{
    lv_coord_t icon_x = lv_obj_get_x(icon[current_max_icon_idx].obj);
    lv_coord_t icon_y = lv_obj_get_y(icon[current_max_icon_idx].obj);
    lv_coord_t icon_w = lv_obj_get_width(icon[current_max_icon_idx].obj);
    lv_coord_t icon_h = lv_obj_get_height(icon[current_max_icon_idx].obj);
    printf("--> x %d , y %d\n", x, y);
    printf("--> screen_set :%d-%d , %d-%d \n", icon_x, icon_x + icon_w, icon_y, icon_y + icon_h);
    if (x >= icon_x && x <= (icon_x + icon_w) && y >= icon_y && y <= (icon_y + icon_h)) {
        open_icon_screen(current_max_icon_idx);
    }
}

// 释放事件回调函数
static void  released_cb(lv_event_t * e)
{
    lv_point_t release_pos;
    // 获取释放时的坐标
    lv_indev_get_point(lv_indev_get_act(), &release_pos);

    int32_t offset_x;
    offset_x = 0;
    // 设置触摸状态为未触摸
    touched = false;

    for (int32_t i = 0; i < OPTION_ICON_COUNT; i++) {
        // 如果图标x坐标大于0
        if (icon[i].x > 0) {
            // 根据x坐标与图标距离的关系计算偏移量
            if (icon[i].x % OPTION_ICON_DISTANCE > OPTION_ICON_DISTANCE / 2) {
                offset_x = OPTION_ICON_DISTANCE - icon[i].x % OPTION_ICON_DISTANCE;
            } else {
                offset_x = -icon[i].x % OPTION_ICON_DISTANCE;
            }
            break;
        }
    }

    for (int32_t i = 0; i < OPTION_ICON_COUNT; i++) {
        // 创建动画，使图标回到合适位置
        lv_myanim_creat(icon[i].obj, set_x_cb, t_offset_x > 0 ? 300 + t_offset_x * 5 : 300 - t_offset_x * 5, 0, lv_anim_path_ease_out,
                        icon[i].x, icon[i].x + offset_x + t_offset_x / 20 * OPTION_ICON_DISTANCE, 0, 0);
        // 更新图标x坐标
        icon[i].x += offset_x + t_offset_x / 20 * OPTION_ICON_DISTANCE;
        // 处理图标x坐标超出范围的情况（循环滚动）
        while (icon[i].x < (-OPTION_ICON_COUNT / 2) * OPTION_ICON_DISTANCE) {
            icon[i].x += (OPTION_ICON_COUNT) * OPTION_ICON_DISTANCE;
        }
        while (icon[i].x >  (OPTION_ICON_COUNT / 2) * OPTION_ICON_DISTANCE) {
            icon[i].x -= (OPTION_ICON_COUNT) * OPTION_ICON_DISTANCE;
        }
    }

    // 更新当前最大图标索引
    update_max_icon_status();
    if (current_max_icon_idx != -1) {
        lv_open_icon_screen_set(release_pos.x, release_pos.y);
    }

}


// 自定义动画创建函数
static void lv_myanim_creat(void * var, lv_anim_exec_xcb_t exec_cb, uint32_t time, uint32_t delay, lv_anim_path_cb_t path_cb,
                            int32_t start, int32_t end, lv_anim_ready_cb_t completed_cb, lv_anim_deleted_cb_t deleted_cb)
{
    lv_anim_t xxx;
    // 初始化动画对象
    lv_anim_init(&xxx);
    // 设置动画对象的变量
    lv_anim_set_var(&xxx, var);
    // 设置动画执行回调函数
    lv_anim_set_exec_cb(&xxx, exec_cb);
    // 设置动画时间
    lv_anim_set_time(&xxx, time);
    // 设置动画延迟
    lv_anim_set_delay(&xxx, delay);
    // 设置动画的起始值和结束值
    lv_anim_set_values(&xxx, start, end);
    // 如果有路径回调函数，设置路径回调函数
    if (path_cb) {
        lv_anim_set_path_cb(&xxx, path_cb);
    }
    // 如果有动画完成回调函数，设置动画完成回调函数
    if (completed_cb) {
        lv_anim_set_ready_cb(&xxx, completed_cb);
    }
    // 如果有动画删除回调函数，设置动画删除回调函数
    if (deleted_cb) {
        lv_anim_set_deleted_cb(&xxx, deleted_cb);
    }
    // 启动动画
    lv_anim_start(&xxx);
}

// 设置x坐标回调函数
// 设置x坐标回调函数
static void set_x_cb(void * var, int32_t v)
{
    // 处理x坐标超出范围的情况（循环滚动）
    while (v < (-OPTION_ICON_COUNT / 2) * OPTION_ICON_DISTANCE) {
        v += (OPTION_ICON_COUNT) * OPTION_ICON_DISTANCE;
    }
    while (v > (OPTION_ICON_COUNT / 2) * OPTION_ICON_DISTANCE) {
        v -= (OPTION_ICON_COUNT) * OPTION_ICON_DISTANCE;
    }

    // 设置对象的x坐标
    lv_obj_set_x(var, v);

    // 获取图标结构体指针
    option_icon_typedef * xxx = (option_icon_typedef *)(((lv_obj_t *)var)->user_data);
    // 更新图标结构体中的x坐标
    xxx->x = v;

    int32_t v_abs = v >= 0 ? v : -v;
    int32_t zoom;
    bool was_max = xxx->is_max; // 保存之前的is_max状态

    // 如果x坐标为0，设置图标为大尺寸
    if (v == 0) {
        zoom = OPTION_ICON_ZOOM_BIG;
        xxx->is_max = true;
    }
    // 如果x坐标超出一定范围，设置图标为小尺寸
    else if (v >= OPTION_ICON_DISTANCE || v <= -OPTION_ICON_DISTANCE) {
        zoom = OPTION_ICON_ZOOM_SMALL;
        xxx->is_max = false;
        lv_obj_set_style_shadow_width(var, 0, LV_PART_MAIN);
    }
    // 其他情况，根据x坐标计算图标尺寸
    else {
        zoom = OPTION_ICON_ZOOM_SMALL + (float)(OPTION_ICON_DISTANCE - v_abs) / OPTION_ICON_DISTANCE * (OPTION_ICON_ZOOM_BIG - OPTION_ICON_ZOOM_SMALL);
        xxx->is_max = (zoom >= OPTION_MAX_ICON_THRESHOLD);
    }

    lv_img_set_zoom(var, zoom);

    // 更新图标名称标签
    if (v >= -OPTION_ICON_MOVE && v <= OPTION_ICON_MOVE) { // 只在中间区域更新名称
        lv_label_set_text(icon_name_lab, xxx->name_lab);
    }
    // 更新当前最大图标索引
    update_max_icon_status();
}

// 新增：更新最大图标状态
static void update_max_icon_status(void)
{
    for (int32_t i = 0; i < OPTION_ICON_COUNT; i++) {
        if (icon[i].is_max) {
            current_max_icon_idx = i;
            // 可以在这里添加额外的逻辑，比如打印日志输出当前最大图标
            // printf("当前最大图标: %s (索引: %d)\n", icon[i].name_lab, i);
            return;
        }
    }
}

// 新增：打开对应屏幕函数
static void open_icon_screen(int32_t icon_idx)
{
    // 根据不同的图标索引打开不同的屏幕
    switch (icon_idx) {
    case 0: //
        printf("点击进入倒计时页面");
        switch_to_timer_page();
        break;
    case 1: //
        printf("点击进入闹钟页面");
        switch_to_alarm_page();
        break;
    case 2: //
        printf("点击进入音乐播放页面");
        switch_to_music_page("net_music");
        break;
    case 3: //
        printf("点击进入对话页面");
        switch_to_conversation_page();
        break;
    case 4: //
        printf("点击进入设置页面");

        break;
    default:
        break;
    }

}


