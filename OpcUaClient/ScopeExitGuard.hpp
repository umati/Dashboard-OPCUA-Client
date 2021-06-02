class ScopeExitGuard
{
public:
    ScopeExitGuard(std::function<void(void)> onExit)
        : m_onExit(onExit)
    {
    }
    ~ScopeExitGuard()
    {
        if (m_onExit)
            m_onExit();
    }
private:
    std::function<void(void)> m_onExit;
};