
namespace SharpTerminal
{
    public interface IConsole
    {        
        void ProcessCmd(params string[] vargs);

        void Println(string text);
        void Clear();
    }
}
