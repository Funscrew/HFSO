

// CLANKER:
// This is an OK way to do a thunk.  Need to dig through moonbounce code as I have a cleaner one in there....
template<typename TObject, typename TFunction, typename... TArgs>
class CThunk
{
public:
    using Method = TFunction (TObject::*)(TArgs...);
    using Function = TFunction (*)(TArgs...);

private:
    static TObject* Instance;
    static Method Target;

    static TFunction CreateFunctionPointer(TArgs... args)
    {
        return (Instance->*Target)(args...);
    }

public:
    static Function Bind(TObject* instance, Method method)
    {
        Instance = instance;
        Target = method;

        return &CreateFunctionPointer;
    }
};


template<typename TObject, typename TFunction, typename... TArgs>
TObject* CThunk<TObject, TFunction, TArgs...>::Instance = nullptr;

template<typename TObject, typename TFunction, typename... TArgs>
typename CThunk<TObject, TFunction, TArgs...>::Method
CThunk<TObject, TFunction, TArgs...>::Target = nullptr;