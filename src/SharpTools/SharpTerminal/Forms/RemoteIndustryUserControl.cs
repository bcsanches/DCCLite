using System;
using System.ComponentModel;
using System.Runtime.Versioning;
using System.Windows.Forms;

namespace SharpTerminal.Forms
{
	[SupportedOSPlatform("windows")]
	public partial class RemoteIndustryUserControl : UserControl
	{
		Tycoon.RemoteIndustry mIndustry;
		IConsole mConsole;

		public RemoteIndustryUserControl()
		{
			InitializeComponent();
		}

		public RemoteIndustryUserControl(IConsole console, Tycoon.RemoteIndustry industry):
			this()
		{
			mConsole = console ?? throw new ArgumentNullException(nameof(console));
			mIndustry = industry ?? throw new ArgumentNullException(nameof(industry));

			m_lbTitle.Text += " " + industry.Name;

			m_tbCargo.Text = industry.CargoName;
			m_tbDailyRate.Text = industry.DailyRate.ToString();
			m_tbQuantity.Text = industry.MaximumQuantity.ToString();

			m_tbStatus.Text = industry.Producing ? "Next batch at " + industry.NextProductionAt : "Not producing";

			mIndustry.PropertyChanged += Industry_PropertyChanged;
		}

		private void Industry_PropertyChanged(object sender, PropertyChangedEventArgs e)
		{
			switch (e.PropertyName)
			{
				case nameof(Tycoon.RemoteIndustry.CurrentQuantity):
					m_tbQuantity.Text = mIndustry.CurrentQuantity.ToString();
					break;

				case nameof(Tycoon.RemoteIndustry.Producing):
				case nameof(Tycoon.RemoteIndustry.NextProductionAt):
					m_tbStatus.Text = mIndustry.Producing ? "Next batch at " + mIndustry.NextProductionAt : "Not producing";
					break;
			}
		}		
	}
}
