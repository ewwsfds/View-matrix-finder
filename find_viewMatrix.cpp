#include <Windows.h>
#include <glm/glm.hpp>
#include <iostream>
using namespace std;
#include <vector>
#include <string>

#include <sstream>

#include <glm/gtc/matrix_transform.hpp>

///Globals
///Globals
///Globals
DWORD pid = 35096;

//Where the entity appears on screen in Pixels
int ScreenPos_startX = 942;
int ScreenPos_startY = 78;

// The width and height to form a rectangle around it in Pixels
int screen_width = 200;
int screen_height = 200;

// The title of the target window to search for
// IMPORTANT: This is the window title displayed in the title bar, NOT the process filename (e.g., "First Triangle" not "game.exe")
HWND hwnd = FindWindowA(nullptr, "First Triangle");

float  fov = 45;
float scale = 1.5;


// Entity Position
glm::vec3 aPos = glm::vec3(0, 0, -5.0f);



// List of All target ViewMatrix addresses
std::vector<uintptr_t> ViewMatrix_addresses = { 0x000000E83891EE30 };




/// END of Globals
/// END of Globals
/// END of Globals









glm::mat4 CreateProjectionMatrix(float fov, float width, float height)
{
    return glm::perspective(
        glm::radians(fov),
        width / height,
        1.0f,   // near plane
        100.0f  // far plane
    );
}


struct ScreenPos
{
    float x;
    float y;
};

// Row-vector math, Y not flipped
ScreenPos world2Screen_row_vectorMath_Yup(
    const glm::mat4& view,
    const glm::mat4& projection,
    const glm::vec3& aPos,
    float width,
    float height)
{
    glm::vec4 clip = glm::vec4(aPos, 1.0f) * view * projection;

    if (clip.w <= 0.0f)
        return { -1.0f, -1.0f };


    glm::vec3 ndc = glm::vec3(clip) / clip.w;

    ScreenPos result;
    result.x = (ndc.x + 1.0f) * width * 0.5f;
    result.y = (ndc.y + 1.0f) * height * 0.5f;

    return result;
}

// Row-vector math, Y flipped
ScreenPos world2Screen_row_vectorMath_Ydown(
    const glm::mat4& view,
    const glm::mat4& projection,
    const glm::vec3& aPos,
    float width,
    float height)
{
    glm::vec4 clip = glm::vec4(aPos, 1.0f) * view * projection;

    if (clip.w <= 0.0f)
        return { -1.0f, -1.0f };


    glm::vec3 ndc = glm::vec3(clip) / clip.w;

    ScreenPos result;
    result.x = (ndc.x + 1.0f) * width * 0.5f;
    result.y = height - ((ndc.y + 1.0f) * height * 0.5f);

    return result;
}

// Column-vector math, Y not flipped
ScreenPos world2Screen_coloumn_vectorMath_Yup(
    const glm::mat4& view,
    const glm::mat4& projection,
    const glm::vec3& aPos,
    float width,
    float height)
{
    glm::vec4 clip = projection * view * glm::vec4(aPos, 1.0f);


    if (clip.w <= 0.0f)
        return { -1.0f, -1.0f };

    glm::vec3 ndc = glm::vec3(clip) / clip.w;

    ScreenPos result;
    result.x = (ndc.x + 1.0f) * width * 0.5f;
    result.y = (ndc.y + 1.0f) * height * 0.5f;

    return result;
}

// Column-vector math, Y flipped
ScreenPos world2Screen_coloumn_vectorMath_Ydown(
    const glm::mat4& view,
    const glm::mat4& projection,
    const glm::vec3& aPos,
    float width,
    float height)
{
    glm::vec4 clip = projection * view * glm::vec4(aPos, 1.0f);

    if (clip.w <= 0.0f)
        return { -1.0f, -1.0f };


    glm::vec3 ndc = glm::vec3(clip) / clip.w;


    ScreenPos result;
    result.x = (ndc.x + 1.0f) * width * 0.5f;
    result.y = height - ((ndc.y + 1.0f) * height * 0.5f);

    return result;
}

glm::mat4 ReadViewMatrix(HANDLE process, uintptr_t address, int offset)
{
    glm::mat4 viewMatrix{};

    ReadProcessMemory(
        process,
        reinterpret_cast<LPCVOID>(address + (offset*4)),
        &viewMatrix,
        sizeof(glm::mat4),
        nullptr
    );

    return viewMatrix;
}



struct OffsetResult
{
    std::string text;
    bool isInside;
};


// Y-Down (DirectX style - origin top-left)
OffsetResult checkScreenOffset_YDown(
    ScreenPos& screen,
    float startX,
    float startY,
    float boxWidth,
    float boxHeight,
    uintptr_t address,
    int offset,
    const std::string& type)
{
    float offsetX = std::abs(screen.x - startX);
    float offsetY = std::abs(screen.y - startY);

    bool isInside = (offsetX <= boxWidth && offsetY <= boxHeight);

    uintptr_t finalAddress = address + (offset * 4);

    char buffer[32];
    sprintf_s(buffer, "0x%llX", static_cast<unsigned long long>(finalAddress));
    std::string addressStr = buffer;

    std::string result =
        "0x" + addressStr +
        " | Pos: (" + std::to_string(int(screen.x)) + ", " + std::to_string(int(screen.y)) + ")" +
        " | offset from Cent: (" + std::to_string(int(offsetX)) + ", " + std::to_string(int(offsetY)) + ")" +
        " | Type: " + type +
        " | is Inside: " + (isInside ? "true" : "false");

    return { result, isInside };
}

// Y-Up (OpenGL style - origin bottom-left)
OffsetResult checkScreenOffset_YUp(
    ScreenPos& screen,
    float startX,
    float startY,
    float boxWidth,
    float boxHeight,
    float screenHeight,   // <-- NEW: actual screen height for flipping
    uintptr_t address,
    int offset,
    const std::string& type)
{
    // Flip Y to convert from OpenGL (bottom-left) to DirectX (top-left)
    float flippedY = screenHeight - screen.y;

    float offsetX = std::abs(screen.x - startX);
    float offsetY = std::abs(flippedY - startY);

    bool isInside = (offsetX <= boxWidth && offsetY <= boxHeight);

    uintptr_t finalAddress = address + (offset * 4);

    char buffer[32];
    sprintf_s(buffer, "0x%llX", static_cast<unsigned long long>(finalAddress));
    std::string addressStr = buffer;

    std::string result =
        "0x" + addressStr +
        " | Pos: (" + std::to_string(int(screen.x)) + ", " + std::to_string(int(screen.y)) + ")" +
        " | Flipped Y: (" + std::to_string(int(flippedY)) + ")" +
        " | offset from Cent: (" + std::to_string(int(offsetX)) + ", " + std::to_string(int(offsetY)) + ")" +
        " | Type: " + type +
        " | is Inside: " + (isInside ? "true" : "false");

    return { result, isInside };
}



HANDLE OpenProcessByPid(DWORD pid)
{
    return OpenProcess(
        PROCESS_VM_READ |
        PROCESS_VM_WRITE |
        PROCESS_VM_OPERATION |
        PROCESS_QUERY_INFORMATION,
        FALSE,
        pid
    );
}


int startX = ScreenPos_startX;
int startY = ScreenPos_startY;

struct WindowSize
{
    float width;
    float height;
};

WindowSize GetWindowSize(HWND hwnd)
{
    RECT rect{};

    GetClientRect(hwnd, &rect);

    float width = static_cast<float>(rect.right - rect.left);
    float height = static_cast<float>(rect.bottom - rect.top);

    return { width, height };
}


int main()
{

    WindowSize size = GetWindowSize(hwnd);

    size.height *= scale;
    size.width *= scale;



    HANDLE handle= OpenProcessByPid(pid);
    if (!handle)
    {
        cout <<"couldnt open process" << endl;
        return 0;
    }





    glm::mat4 projectionMatrix = CreateProjectionMatrix(fov, size.width, size.height);

    vector<OffsetResult> validMatrices;

    for (const uintptr_t& view_address : ViewMatrix_addresses)
    {
        for (int i = -8; i <= 8; i++)
        {
            glm::mat4 view_Matrix = ReadViewMatrix(handle, view_address, i);

            if (view_address == 0)
            {
                continue;
            }



            ScreenPos screen_cord_row_Yup = world2Screen_row_vectorMath_Yup(view_Matrix, projectionMatrix, aPos, size.width, size.height);
            ScreenPos screen_cord_row_Ydown = world2Screen_row_vectorMath_Ydown(view_Matrix, projectionMatrix, aPos, size.width, size.height);
            ScreenPos screen_cord_coloumn_Yup = world2Screen_coloumn_vectorMath_Yup(view_Matrix, projectionMatrix, aPos, size.width, size.height);
            ScreenPos screen_cord_coloumn_Ydown = world2Screen_coloumn_vectorMath_Ydown(view_Matrix, projectionMatrix, aPos, size.width, size.height);



            OffsetResult result_cord_row_Yup = checkScreenOffset_YUp(screen_cord_row_Yup, startX, startY, screen_width, screen_height, size.height, view_address, i, "Rw_Yup");
            OffsetResult result_cord_row_Ydown = checkScreenOffset_YDown(screen_cord_row_Ydown, startX, startY, screen_width, screen_height, view_address, i, "Rw_Ydown");

            OffsetResult result_cord_coloumn_Yup = checkScreenOffset_YUp(screen_cord_coloumn_Yup, startX, startY, screen_width, screen_height, size.height, view_address, i, "Clmn_Yup");
            OffsetResult result_cord_coloumn_Ydown = checkScreenOffset_YDown(screen_cord_coloumn_Ydown, startX, startY, screen_width, screen_height, view_address, i, "Clmn_Ydown");


            if (result_cord_row_Yup.isInside)
            {
                validMatrices.push_back(result_cord_row_Yup);

            }

            if (result_cord_row_Ydown.isInside)
            {
                validMatrices.push_back(result_cord_row_Ydown);

            }


            if (result_cord_coloumn_Yup.isInside)
            {
                validMatrices.push_back(result_cord_coloumn_Yup);

            }


            if (result_cord_coloumn_Ydown.isInside)
            {
                validMatrices.push_back(result_cord_coloumn_Ydown);

            }

            cout << result_cord_row_Yup.text << endl;
            cout << result_cord_row_Ydown.text << endl;
            cout << result_cord_coloumn_Yup.text << endl;
            cout << result_cord_coloumn_Ydown.text << endl;

        }
    }

    cout << "_____________________________________________" << endl;
    cout << "______________ VALID Matrices _______________" << endl;
    cout << "_____________________________________________" << endl;


    for (const OffsetResult& Valid_view : validMatrices)
    {
        cout << Valid_view.text << endl;

    }


    cout << "Screen Width: " << size.width << " Height: " << size.height<< endl;


}
