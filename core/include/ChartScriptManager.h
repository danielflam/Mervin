#ifndef ChartScriptFactoryManager_H
#define ChartScriptFactoryManager_H


namespace indicator
{

//////////////////////////////////////////////
//
//  ChartScriptFactoryManager
//
//////////////////////////////////////////////

	class ChartScriptFactoryManager : public boost::noncopyable
	{
	private:
		ChartScriptFactoryManager();
		virtual ~ChartScriptFactoryManager();

	public:
		
		static ChartScriptFactoryManager & getInstance();

		// The following semantics are designed for interfacing with script languages
		void registerScript( string scriptName, ChartScriptFactoryBase * factory );
		ChartScript * createScriptInstance(string scriptName, 
			TradingSession * session, 
			ChartScript::ChartScriptOptions options = ChartScript::CSO_DEFAULT);
	protected:
		ChartScriptFactoryMap chartScriptFactories;

	private:
		int lastscriptid;
	};

	template <class T>
	class RegisterClassScript
	{
	public:
		RegisterClassScript(string scriptname)
		{
			ChartScriptFactoryManager::getInstance().registerScript(scriptname, new ChartScriptFactory<T>);
		}
	};

}


#endif