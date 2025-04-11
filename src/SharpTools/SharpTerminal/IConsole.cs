
using System.Json;
using System.Threading.Tasks;

namespace SharpTerminal
{
    public interface IConsole
    {        
        int ProcessCmd(params object[] vargs);
        int ProcessCmd(IResponseHandler handler, params object[] vargs);

        Task<JsonValue> RequestAsync(params object[] vargs);

        void HandleReadEEPromResult(JsonObject responseObj);


		void Println(string text);
        void Clear();
    }
}
