#include <iostream>
#include <chrono>
#include <thread>
#include <signal.h>
#include <mutex>
#include <cstdarg>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <jni.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <cmath>

// Assuming these are defined somewhere
struct Vector3 {
	float x, y, z;
	float distanceTo(const Vector3& other) const {
		return sqrt((x - other.x) * (x - other.x) + 
					(y - other.y) * (y - other.y) + 
					(z - other.z) * (z - other.z));
	}
};

struct Matrix4x4 {
	// Matrix data
};

class Driver {
public:
	bool initialize(pid_t pid) {
		// Initialization code
		return true;
	}
	void deinitialize() {
		// Deinitialization code
	}
	template<typename T>
	T read(uintptr_t address) {
		// Read memory code
		return T();
	}
	uintptr_t get_module_base(const char* module_name) {
		// Get module base address code
		return 0;
	}
};

void UpdatePlayerPosition() {
	// Update player position code
}

uintptr_t GetActorArray() {
	// Get actor array code
	return 0;
}

void DrawPlayerBox(const Vector3& position, const Matrix4x4& viewMatrix) {
	// Draw player box code
}

// Global variables
bool keepRunning = true;
std::mutex positionMutex;
Vector3 playerPosition;
int screenWidth = 1080; // Updated screen width for Realme 9 SE
int screenHeight = 2400; // Updated screen height for Realme 9 SE
Driver* driver = nullptr; // Ensure driver is defined

void intHandler(int signal) {
	keepRunning = false; // Stop the loop on CTRL+C
}

// Function to log messages
void Log(const char* format, ...) {
	va_list args;
	va_start(args, format);
	__android_log_vprint(ANDROID_LOG_INFO, "ESP", format, args);
	va_end(args);
}

EGLDisplay display;
EGLSurface surface;
EGLContext context;

bool initializeEGL(ANativeWindow* window) {
	display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (display == EGL_NO_DISPLAY) {
		Log("Failed to get EGL display");
		return false;
	}

	if (!eglInitialize(display, nullptr, nullptr)) {
		Log("Failed to initialize EGL");
		return false;
	}

	EGLConfig config;
	EGLint numConfigs;
	EGLint configAttribs[] = {
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_DEPTH_SIZE, 24,
		EGL_NONE
	};

	if (!eglChooseConfig(display, configAttribs, &config, 1, &numConfigs)) {
		Log("Failed to choose EGL config");
		return false;
	}

	surface = eglCreateWindowSurface(display, config, window, nullptr);
	if (surface == EGL_NO_SURFACE) {
		Log("Failed to create EGL window surface");
		return false;
	}

	EGLint contextAttribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
	if (context == EGL_NO_CONTEXT) {
		Log("Failed to create EGL context");
		return false;
	}

	if (!eglMakeCurrent(display, surface, surface, context)) {
		Log("Failed to make EGL context current");
		return false;
	}

	return true;
}

void cleanupEGL() {
	if (display != EGL_NO_DISPLAY) {
		eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (context != EGL_NO_CONTEXT) {
			eglDestroyContext(display, context);
		}
		if (surface != EGL_NO_SURFACE) {
			eglDestroySurface(display, surface);
		}
		eglTerminate(display);
	}
	display = EGL_NO_DISPLAY;
	context = EGL_NO_CONTEXT;
	surface = EGL_NO_SURFACE;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_esp_MainActivity_nativeInit(JNIEnv* env, jobject /* this */, jobject surface) {
	signal(SIGINT, intHandler); // Catch CTRL+C to exit

	pid_t pid = get_name_pid((char *)"com.pubg.imobile");
	if (pid <= 0) {
		Log("Failed to get process ID.\n");
		return; // Exit if the process is not found
	}
	Log("pid = %d\n", pid);

	// Initialize the driver first
	driver = new Driver(); // Ensure driver is allocated
	if (!driver->initialize(pid)) {
		Log("Failed to initialize driver.\n");
		delete driver; // Clean up
		return; // Exit if driver initialization fails
	}

	ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
	if (!initializeEGL(window)) {
		Log("Failed to initialize EGL");
		delete driver; // Clean up
		return;
	}

	// Main loop for continuous updating
	while (keepRunning) {
		// Update player position for visibility checks
		UpdatePlayerPosition();

		uintptr_t actorArray = GetActorArray();
		int actorCount = driver->read<int>(actorArray + ActorCountOffset);

		// Ensure valid actor array and count
		if (actorArray == 0 || actorCount <= 0 || actorCount > 1000) {
			Log("Invalid actor array or count.\n");
			continue; // Skip this iteration
		}

		// Read the view matrix
		Matrix4x4 viewMatrix = driver->read<Matrix4x4>(driver->get_module_base("libUE4.so") + ViewMatrix);

		// Clear the screen and update frame
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Iterate through actors and draw boxes
		for (int i = 0; i < actorCount; ++i) {
			uintptr_t actor = driver->read<uintptr_t>(actorArray + i * sizeof(uintptr_t));
			if (actor == 0) continue; // Skip invalid actors

			Vector3 position = driver->read<Vector3>(actor + PositionOffset);
			if (position.x == 0 && position.y == 0 && position.z == 0) continue; // Skip if position is invalid

			// Check if actor is visible
			{
				std::lock_guard<std::mutex> lock(positionMutex);
				if (position.distanceTo(playerPosition) <= MAX_ACTOR_DISTANCE) {
					DrawPlayerBox(position, viewMatrix);
					Log("Drawing box for actor %d at position: (%f, %f, %f)\n", i, position.x, position.y, position.z);
				}
			}
		}

		// Swap buffers and poll for events
		eglSwapBuffers(display, surface);
	}

	// Cleanup code here
	if (driver) {
		driver->deinitialize(); // Assuming there is a deinitialize method
		delete driver; // Clean up driver memory
	}

	cleanupEGL();
	ANativeWindow_release(window);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_esp_MainActivity_nativeStop(JNIEnv* env, jobject /* this */) {
	keepRunning = false; // Signal the main loop to stop
}
