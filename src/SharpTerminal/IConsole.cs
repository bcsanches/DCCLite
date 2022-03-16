
using System.Json;
using System.Threading.Tasks;

namespace SharpTerminal
{
    public interface IConsole
    {        
        int ProcessCmd(params string[] vargs);
        int ProcessCmd(IResponseHandler handler, params string[] vargs);

        Task<JsonValue> RequestAsync(params string[] vargs);

        void Println(string text);
        void Clear();
    }
}
