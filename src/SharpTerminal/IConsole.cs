
namespace SharpTerminal
{
    public interface IConsole
    {
        void ProcessCmd(string[] vargs);

        void Println(string text);
        void Clear();
    }
}
