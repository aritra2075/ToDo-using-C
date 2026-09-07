#include<gtk/gtk.h>
#include<stdio.h>
#include<string.h>
#include<time.h>

#define TODO_FILE "todo.csv"
#define TEMP_FILE "todo.tmp"

typedef struct {
    GtkWidget *task_list;
    GtkWidget *task_entry;
    GtkWidget *status_label;
} TodoApp;

static void load_tasks(TodoApp *app);

static void set_status(TodoApp *app, const char *message)
{
    gtk_label_set_text(GTK_LABEL(app->status_label), message);
}

static gboolean ensure_file(void)
{
    FILE *file = fopen(TODO_FILE, "r");
    if (file != NULL) {
        fclose(file);
        return TRUE;
    }

    file = fopen(TODO_FILE, "w");
    if (file == NULL) {
        return FALSE;
    }

    fputs("Day-Month-DATE,TIME-Year,TO_DO\n", file);
    fclose(file);
    return TRUE;
}

static void clear_task_list(TodoApp *app)
{
    GtkWidget *child = gtk_widget_get_first_child(app->task_list);
    while (child != NULL) {
        GtkWidget *next = gtk_widget_get_next_sibling(child);
        gtk_list_box_remove(GTK_LIST_BOX(app->task_list), child);
        child = next;
    }
}

static void delete_task(GtkButton *button, gpointer user_data)
{
    TodoApp *app = user_data;
    int selected_task = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button), "task-number"));
    FILE *input = fopen(TODO_FILE, "r");
    FILE *temporary = fopen(TEMP_FILE, "w");
    char line[1024];
    int current_task = 0;

    if (input == NULL || temporary == NULL) {
        if (input != NULL) fclose(input);
        if (temporary != NULL) fclose(temporary);
        remove(TEMP_FILE);
        set_status(app, "Could not open the task file.");
        return;
    }

    if (fgets(line, sizeof(line), input) != NULL) {
        fputs(line, temporary);
    }

    while (fgets(line, sizeof(line), input) != NULL) {
        current_task++;
        if (current_task != selected_task) {
            fputs(line, temporary);
        }
    }

    fclose(input);
    fclose(temporary);

    if (remove(TODO_FILE) != 0 || rename(TEMP_FILE, TODO_FILE) != 0) {
        remove(TEMP_FILE);
        set_status(app, "Could not update the task file.");
        return;
    }

    set_status(app, "Task deleted.");
    load_tasks(app);
}

static void load_tasks(TodoApp *app)
{
    FILE *file = fopen(TODO_FILE, "r");
    char line[1024];
    int task_number = 0;

    clear_task_list(app);
    if (file == NULL) {
        set_status(app, "Could not open the task file.");
        return;
    }

    fgets(line, sizeof(line), file);
    while (fgets(line, sizeof(line), file) != NULL) {
        GtkWidget *row;
        GtkWidget *label;
        GtkWidget *delete_button;
        GtkWidget *content;
        size_t length = strlen(line);

        if (length > 0 && line[length - 1] == '\n') {
            line[length - 1] = '\0';
        }

        task_number++;
        row = gtk_list_box_row_new();
        content = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
        label = gtk_label_new(line);
        delete_button = gtk_button_new_with_label("Delete");

        gtk_widget_add_css_class(row, "task-row");
        gtk_widget_add_css_class(delete_button, "delete-button");
        gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
        gtk_label_set_wrap(GTK_LABEL(label), TRUE);
        gtk_widget_set_hexpand(label, TRUE);
        gtk_widget_set_margin_top(content, 12);
        gtk_widget_set_margin_bottom(content, 12);
        gtk_widget_set_margin_start(content, 16);
        gtk_widget_set_margin_end(content, 16);
        gtk_box_append(GTK_BOX(content), label);
        gtk_box_append(GTK_BOX(content), delete_button);
        gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), content);
        g_object_set_data(G_OBJECT(delete_button), "task-number", GINT_TO_POINTER(task_number));
        g_signal_connect(delete_button, "clicked", G_CALLBACK(delete_task), app);
        gtk_list_box_append(GTK_LIST_BOX(app->task_list), row);
    }

    fclose(file);
    if (task_number == 0) {
        set_status(app, "No tasks yet. Add one above.");
    }
}

static void add_task(GtkButton *button, gpointer user_data)
{
    TodoApp *app = user_data;
    const char *task = gtk_editable_get_text(GTK_EDITABLE(app->task_entry));
    time_t current_time = time(NULL);
    struct tm *local_time = localtime(&current_time);
    char timestamp[64];
    FILE *file;

    (void)button;
    if (task[0] == '\0') {
        set_status(app, "Write a task before adding it.");
        return;
    }

    if (local_time == NULL || strftime(timestamp, sizeof(timestamp), "%a %b %d %H:%M:%S %Y", local_time) == 0) {
        set_status(app, "Could not read the current time.");
        return;
    }

    file = fopen(TODO_FILE, "a");
    if (file == NULL) {
        set_status(app, "Could not save the task.");
        return;
    }

    fprintf(file, "%s,%s\n", timestamp, task);
    fclose(file);
    gtk_editable_set_text(GTK_EDITABLE(app->task_entry), "");
    set_status(app, "Task added.");
    load_tasks(app);
}

static void apply_theme(void)
{
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css =
        "window { background: #1a1b26; color: #c0caf5; }"
        "label { color: #c0caf5; }"
        ".title { color: #7aa2f7; font-size: 28px; font-weight: bold; }"
        ".subtitle { color: #565f89; font-size: 13px; }"
        "entry { background: #24283b; color: #c0caf5; border: 1px solid #414868; border-radius: 8px; padding: 10px; }"
        "entry:focus { border-color: #7aa2f7; }"
        "button { background: #7aa2f7; color: #1a1b26; border-radius: 8px; padding: 10px 18px; font-weight: bold; }"
        "button:hover { background: #89b4fa; }"
        ".delete-button { background: #f7768e; color: #1a1b26; padding: 7px 12px; }"
        ".delete-button:hover { background: #ff9eaa; }"
        ".task-row { background: #24283b; border: 1px solid #2f354f; border-radius: 8px; margin-bottom: 8px; }"
        "list { background: transparent; }"
        ".status { color: #bb9af7; font-size: 13px; }";

    gtk_css_provider_load_from_string(provider, css);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

static void activate(GtkApplication *application, gpointer user_data)
{
    TodoApp *app = user_data;
    GtkWidget *window = gtk_application_window_new(application);
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *input_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *title = gtk_label_new("My Tasks");
    GtkWidget *subtitle = gtk_label_new("A quiet place for the things worth doing.");
    GtkWidget *add_button = gtk_button_new_with_label("Add task");
    GtkWidget *scroll = gtk_scrolled_window_new();

    app->task_list = gtk_list_box_new();
    app->task_entry = gtk_entry_new();
    app->status_label = gtk_label_new("");

    apply_theme();
    gtk_window_set_title(GTK_WINDOW(window), "TODO");
    gtk_window_set_default_size(GTK_WINDOW(window), 720, 560);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
    gtk_widget_set_margin_top(main_box, 32);
    gtk_widget_set_margin_bottom(main_box, 28);
    gtk_widget_set_margin_start(main_box, 36);
    gtk_widget_set_margin_end(main_box, 36);
    gtk_widget_set_margin_bottom(header_box, 24);
    gtk_widget_set_margin_bottom(input_box, 22);
    gtk_widget_set_margin_bottom(app->status_label, 14);
    gtk_widget_set_hexpand(app->task_entry, TRUE);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->task_entry), "What needs doing?");
    gtk_widget_add_css_class(title, "title");
    gtk_widget_add_css_class(subtitle, "subtitle");
    gtk_widget_add_css_class(app->status_label, "status");
    gtk_label_set_xalign(GTK_LABEL(title), 0.0f);
    gtk_label_set_xalign(GTK_LABEL(subtitle), 0.0f);
    gtk_label_set_xalign(GTK_LABEL(app->status_label), 0.0f);
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(app->task_list), GTK_SELECTION_NONE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), app->task_list);
    gtk_box_append(GTK_BOX(header_box), title);
    gtk_box_append(GTK_BOX(header_box), subtitle);
    gtk_box_append(GTK_BOX(input_box), app->task_entry);
    gtk_box_append(GTK_BOX(input_box), add_button);
    gtk_box_append(GTK_BOX(main_box), header_box);
    gtk_box_append(GTK_BOX(main_box), input_box);
    gtk_box_append(GTK_BOX(main_box), app->status_label);
    gtk_box_append(GTK_BOX(main_box), scroll);
    gtk_window_set_child(GTK_WINDOW(window), main_box);
    g_signal_connect(add_button, "clicked", G_CALLBACK(add_task), app);
    g_signal_connect(app->task_entry, "activate", G_CALLBACK(add_task), app);
    gtk_window_present(GTK_WINDOW(window));
    load_tasks(app);
}

int main(int argc, char **argv)
{
    GtkApplication *application;
    TodoApp app = {0};
    int status;

    if (!ensure_file()) {
        fprintf(stderr, "Could not create %s\n", TODO_FILE);
        return 1;
    }

    application = gtk_application_new("com.example.todo", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(application, "activate", G_CALLBACK(activate), &app);
    status = g_application_run(G_APPLICATION(application), argc, argv);
    g_object_unref(application);
    return status;
}
