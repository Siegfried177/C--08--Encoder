#include <gtk/gtk.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

const char *vocab = " !\"#$%&'(){}[]*+;,./\\-\n0123456789:<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
char *new_word = NULL, *key = NULL;
int method = 0;
GtkWidget *label_pre, *label_post;

// Función para verificar si la clave son todos digitos
int is_number(char *cadena){
    for (int i = 0; i < strlen(cadena); i++) 
        if (! isdigit(cadena[i])) return 0;
    return 1;
}

// Función para actualizar los labels, con el contenido antes y después de descifrar
void actualizar_labels(const gchar *texto_original, const gchar *texto_cifrado) {
    gtk_label_set_text(GTK_LABEL(label_pre), texto_original);   // Actualizar el texto original
    gtk_label_set_text(GTK_LABEL(label_post), texto_cifrado);     // Actualizar el texto cifrado
}

// Función para encriptar o desencriptar
// Parametros: Palabra a encriptar, palabra encriptada, Clave de Cifrado, 0 = encriptar y 1 = desencriptar
void encode_decode(char *word, char *new_word, char *key, int method){
    char *p = NULL;
    int index, range, c = 0;
    for (int i = 0; i < (int)strlen(word); i++){
        p = strchr(vocab, word[i]);
        range = ! method ? (int)(key[c++] - '0') : - (int)(key[c++] - '0');
        g_print("%d\n", range);
        if (c == (int)strlen(key)) c = 0;
        index = (int)(p - vocab);
        new_word[i] = vocab[index + range];
    }
    new_word[strlen(word)] = '\0';
    actualizar_labels(word, new_word);
}

// Función para manejar el cierre de la ventana modal
static void on_modal_window_response(GtkDialog *dialog, gint response_id, gpointer data) {
    GtkWidget *entry = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "entry"));
    const gchar *text_input = gtk_entry_get_text(GTK_ENTRY(entry));
    key = malloc(strlen(text_input) + 1);
    if(is_number((char*)text_input)) strcpy(key, text_input);
    gtk_widget_destroy(GTK_WIDGET(dialog));

    if (strlen(key) != 0) {
        GtkWidget *file_chooser;
        file_chooser = gtk_file_chooser_dialog_new("Selecciona el archivo a encriptar / desencriptar", GTK_WINDOW(data), GTK_FILE_CHOOSER_ACTION_OPEN, 
                                                    "_Cancelar", GTK_RESPONSE_CANCEL, "_Abrir", GTK_RESPONSE_ACCEPT, NULL);
        if (gtk_dialog_run(GTK_DIALOG(file_chooser)) == GTK_RESPONSE_ACCEPT) {
            gchar *file_name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_chooser));
            FILE *file = fopen(file_name, "r");

            if (file) {
                gchar *file_content = NULL;
                gsize length = 0;
                GError *error = NULL;

                if (g_file_get_contents(file_name, &file_content, &length, &error)) {
                    new_word = malloc(strlen(file_content) + 1);
                    encode_decode(file_content, new_word, key, method);
                    g_free(file_content);
                } 
                fclose(file);
            } 
            else g_print("No se pudo abrir el archivo.\n");
            g_free(file_name);
        }
        gtk_widget_destroy(file_chooser);
    }
}

// Función para mostrar la ventana modal al hacer clic en el botón
static void show_modal(GtkWidget *button, gpointer data) {
    GtkWidget *modal_window;
    GtkWidget *entry;
    const char *button_label = gtk_button_get_label(GTK_BUTTON(button));
    if (! strcmp(button_label, "Desencriptar")) method = 1;
    else method = 0;
    modal_window = gtk_dialog_new_with_buttons("Ingresar Clave", GTK_WINDOW(data), GTK_DIALOG_MODAL, "_Ingresar", GTK_RESPONSE_ACCEPT, NULL);

    entry = gtk_entry_new();
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(modal_window));
    gtk_container_add(GTK_CONTAINER(content_area), entry);
    g_object_set_data(G_OBJECT(modal_window), "entry", entry);
    g_signal_connect(modal_window, "response", G_CALLBACK(on_modal_window_response), data);
    gtk_widget_show_all(modal_window);
}

static void on_exit_clicked(GtkWidget *widget, gpointer data){
    gtk_main_quit();
}

int main(int argc, char *argv[]){
    GtkWidget *window;
    gtk_init(&argc, &argv);
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Encoder -- Decoder");
    gtk_window_set_default_size(GTK_WINDOW(window), 450, 300);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *grid = gtk_grid_new();

    for (int i = 0; i < 3; i++) {
            GtkWidget *button;
            if (i == 0){
                button = gtk_button_new_with_label("Encriptar");
                g_signal_connect(button, "clicked", G_CALLBACK(show_modal), window);
            }
            else if (i == 1){
                button = gtk_button_new_with_label("Desencriptar");
                g_signal_connect(button, "clicked", G_CALLBACK(show_modal), window);
            }
            else{
                button = gtk_button_new_with_label("Salir");
                g_signal_connect(button, "clicked", G_CALLBACK(on_exit_clicked), NULL);
            }
            gtk_grid_attach(GTK_GRID(grid), button, i, 0, 1, 1);
            gtk_widget_set_hexpand(button, TRUE);
        }

    gtk_grid_set_row_spacing(GTK_GRID(grid), 20);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    label_pre = gtk_label_new("");
    label_post = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(grid), label_pre, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_post, 1, 2, 1, 1);
    
    gtk_box_pack_start(GTK_BOX(box), grid, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(window), box);
    gtk_widget_show_all(window);

    gtk_main();
    return 0;
}