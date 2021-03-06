/*****************************************************************************
 * DashGL.com GTK GL-Area                                                    *
 * This file is in the public domain                                         *
 * Contributors: Benjamin Collins                                            *
 *****************************************************************************/

#include <GL/glew.h>
#include <gtk/gtk.h>

static void activate(GtkApplication *app, gpointer user_data);
static void on_realize(GtkGLArea *area);
static gboolean on_render(GtkGLArea *area, GdkGLContext *context);

GLuint program;
GLuint vao, vbo_triangle;
GLint attribute_coord2d;

static void open_activated(GtkWidget *f, gpointer user_data);
static void quit_activated(GtkWidget *f, gpointer user_data);

int main(int argc, char *argv[]) {
	
	GtkApplication *app;
	int status;

	app = gtk_application_new("com.termantics.example3", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);

	return status;

}

static void activate(GtkApplication *app, gpointer user_data) {

	GtkWidget *window;
	GtkWidget *box;
	GtkWidget *menubar, *filemenu, *file, *open, *quit;
	GtkWidget *gl_area;

	window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "Window");
	gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(window), box);
	
	menubar = gtk_menu_bar_new();
	filemenu = gtk_menu_new();
	file = gtk_menu_item_new_with_label("File");
	open = gtk_menu_item_new_with_label("Open");
	quit = gtk_menu_item_new_with_label("Quit");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), filemenu);
   	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), open);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quit);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file);

	g_signal_connect(G_OBJECT(open), "activate", G_CALLBACK(open_activated), window);
	g_signal_connect(G_OBJECT(quit), "activate", G_CALLBACK(quit_activated), window);

	gtk_box_pack_start(GTK_BOX(box), menubar, FALSE, FALSE, 0);

	gl_area = gtk_gl_area_new();
	gtk_widget_set_vexpand(gl_area, TRUE);
	gtk_widget_set_hexpand(gl_area, TRUE);
	g_signal_connect (gl_area, "realize", G_CALLBACK(on_realize), NULL);
	g_signal_connect (gl_area, "render", G_CALLBACK(on_render), NULL);
	gtk_container_add(GTK_CONTAINER(box), gl_area);

	gtk_widget_show_all(window);

}

static void on_realize(GtkGLArea *area) {

	g_print("on_realize\n");
	
	gtk_gl_area_make_current(area);
	
	if (gtk_gl_area_get_error (area) != NULL) {
		fprintf(stderr, "Unknown error\n");
		return;
	}

	glewExperimental = GL_TRUE;
	glewInit();

	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	gtk_gl_area_set_has_depth_buffer(area, TRUE);
	
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

    GLfloat triangle_vertices[] = {
         0.0,  0.8,
        -0.8, -0.8,
         0.8, -0.8
    };
    glGenBuffers(1, &vbo_triangle);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(triangle_vertices),
        triangle_vertices,
        GL_STATIC_DRAW
    );
	
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glDisableVertexAttribArray(0);

    GLint compile_ok = GL_FALSE;
    GLint link_ok = GL_FALSE;

    const char *vs_source =
    "#version 130\n" // OpenGL 3
    "attribute vec2 coord2d; \n"
    "void main (void) { \n"
    "   gl_Position = vec4(coord2d, 0.0, 1.0); \n"
    "}";

    const char *fs_source =
    "#version 130\n" // OpenGL 3
    "void main (void) {\n"
    "   gl_FragColor[0] = 0.0;\n"
    "   gl_FragColor[1] = 0.0;\n"
    "   gl_FragColor[2] = 1.0;\n"
    "}";

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fs_source, NULL);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &compile_ok);
    if(!compile_ok) {
        fprintf(stderr, "Error in fragment shader\n");
        return;
    }

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vs_source, NULL);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &compile_ok);
    if(!compile_ok) {
        fprintf(stderr, "Error in vertex shader\n");
        return;
    }

    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if(!link_ok) {
        fprintf(stderr, "Error when linking program\n");
        return;
    }

    const char *attribute_name = "coord2d";
    attribute_coord2d = glGetAttribLocation(program, attribute_name);
    if(attribute_coord2d == -1) {
        fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
        return;
    }


}

static gboolean on_render(GtkGLArea *area, GdkGLContext *context) {

	g_print("on_render\n");
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program);
  
  	glBindVertexArray (vao);
  	glEnableVertexAttribArray(attribute_coord2d);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
    glVertexAttribPointer(
        attribute_coord2d,
        2,
        GL_FLOAT,
        GL_FALSE,
        0,
        0
    );


    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(attribute_coord2d);

	return TRUE;

}

static void open_activated(GtkWidget *f, gpointer window) {

	g_print("File -> Open activated.\n");
	
	GtkWidget *dialog;
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
	gint res;

	dialog = gtk_file_chooser_dialog_new ("Open File", NULL, action,
		"_Cancel", GTK_RESPONSE_CANCEL,
		"_Open", GTK_RESPONSE_ACCEPT, NULL);

	res = gtk_dialog_run (GTK_DIALOG (dialog));

	if (res == GTK_RESPONSE_ACCEPT) {
		char *filename;
		GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
		filename = gtk_file_chooser_get_filename (chooser);
		printf("%s\n", filename);
		//free(filename);
	}
	
	gtk_widget_destroy (dialog);

}

static void quit_activated(GtkWidget *f, gpointer window) {

	// g_application_quit(GTK_APPLICATION(app));
	gtk_widget_destroy(window);

}
