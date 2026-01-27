#include <glad/glad.h>
#include <string>
#include <map>
#include <vector>
#include <imgui.h>

struct GpuTimer
{
    GLuint Queries[2] = {0, 0};
    int QueryIndex = 0;
    float TimeMs = 0.0f;
    ImColor Color;

    GpuTimer()
    {
        glGenQueries(2, Queries);
        Color = ImColor::HSV(static_cast<float>(rand()) / static_cast<float>(RAND_MAX), 0.6f, 0.8f);
    }
};

class RenderProfiler 
{

public:
    static std::map<std::string, GpuTimer>& GetTimerMap()
    {
        static std::map<std::string, GpuTimer> timers;
        return timers;
    }

    static std::vector<std::string>& GetFrameOrder()
    {
        static std::vector<std::string> order;
        return order;
    }

};

struct ProfileScope 
{
    std::string name;
    ProfileScope(const std::string& n) : name(n)
    {
        auto& timer = RenderProfiler::GetTimerMap()[name];
        
        auto& order = RenderProfiler::GetFrameOrder();
        if (std::find(order.begin(), order.end(), name) == order.end())
        {
            order.push_back(name);
        }

        glBeginQuery(GL_TIME_ELAPSED, timer.Queries[timer.QueryIndex]);
    }

    ~ProfileScope()
    {
        auto& timer = RenderProfiler::GetTimerMap()[name];
        glEndQuery(GL_TIME_ELAPSED);

        // fetch previous buffer
        int prevIndex = 1 - timer.QueryIndex;
        GLuint64 timeNs;
        glGetQueryObjectui64v(timer.Queries[prevIndex], GL_QUERY_RESULT, &timeNs);
        timer.TimeMs = timeNs / 1000000.0f;
        
        timer.QueryIndex = prevIndex;
    }
};

#define PROFILE_GPU(name) ProfileScope gpu_scope_##__LINE__(name)