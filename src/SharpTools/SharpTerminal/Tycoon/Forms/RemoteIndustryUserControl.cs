using SharpTerminal.Tycoon;
using SharpTerminal.Tycoon.Forms;
using System;
using System.ComponentModel;
using System.Runtime.Versioning;
using System.Threading.Tasks;
using System.Windows.Forms;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;

namespace SharpTerminal.Forms
{
	[SupportedOSPlatform("windows")]
	public partial class RemoteIndustryUserControl : UserControl
	{
		Tycoon.RemoteIndustry mIndustry;
		Tycoon.Spot mSelectedSpot;
		IConsole mConsole;
		Func<Task> mLinkClick;
		Func<Task> mLinkClickAux;

		public RemoteIndustryUserControl()
		{
			InitializeComponent();
		}

		public RemoteIndustryUserControl(IConsole console, Tycoon.RemoteIndustry industry) :
			this()
		{
			mConsole = console ?? throw new ArgumentNullException(nameof(console));
			mIndustry = industry ?? throw new ArgumentNullException(nameof(industry));

			m_lbTitle.Text += " " + industry.Name;
			
			m_tbDailyRate.Text = industry.DailyRate.ToString();
			
			this.FillProductionListView();

			m_tbStatus.Text = industry.Producing ? "Next batch at " + industry.NextProductionAt : "Not producing";

			mIndustry.PropertyChanged += Industry_PropertyChanged;

			var spots = industry.GetSpots();
			foreach (var spot in spots)
			{
				var index = m_cbSpot.Items.Add(spot);
			}

			m_cbSpot.SelectedIndex = 0;
		}

		private void FillProductionListView()
		{
			var production = mIndustry.Produces;

			m_lvProduction.SuspendLayout();
			foreach(var cargoInfo in production)
			{
				var item = new ListViewItem(cargoInfo.CargoName);

				item.SubItems.Add(cargoInfo.CurrentQuantity.ToString());
				item.SubItems.Add(cargoInfo.ReservedQuantity.ToString());
				item.SubItems.Add((cargoInfo.CurrentQuantity + cargoInfo.ReservedQuantity).ToString());

				m_lvProduction.Items.Add(item);
			}

			foreach (ColumnHeader column in m_lvProduction.Columns)
			{
				column.Width = -2;
			}

			m_lvProduction.ResumeLayout();
		}

#if false
		private void UpdateQuantityInfo()
		{
			var qtd = mIndustry.CurrentQuantity;
			var reserved = mIndustry.ReservedQuantity;

			if(reserved == 0)
			{
				m_tbQuantity.Text = qtd.ToString();
			}
			else
			{
				m_tbQuantity.Text = $"stored {qtd}, reserved {reserved}, total {qtd + reserved}";
			}			
		}
#endif

		private void Industry_PropertyChanged(object sender, PropertyChangedEventArgs e)
		{
			switch (e.PropertyName)
			{
#if false
				case nameof(Tycoon.RemoteIndustry.CurrentQuantity):
				case nameof(Tycoon.RemoteIndustry.ReservedQuantity):
					UpdateQuantityInfo();
					break;					
#endif

				case nameof(Tycoon.RemoteIndustry.Producing):
				case nameof(Tycoon.RemoteIndustry.NextProductionAt):
					m_tbStatus.Text = mIndustry.Producing ? "Next batch at " + mIndustry.NextProductionAt : "Not producing";
					break;
			}
		}

		private void RefreshSpotInfo(Spot spot)
		{
			m_tbSpotInfo.Text = spot.Information;
			m_tbSpotState.Text = spot.State.ToString();

			switch (spot.State)
			{
				case Tycoon.SpotStates.FREE:
					m_lnkSpotAction.Text = "&Reserve";
					m_lnkSpotAction.Visible = true;
					mLinkClick = OnSetIndustrySpotReservedClick;
					break;

				case Tycoon.SpotStates.RESERVED:
					m_lnkSpotAction.Text = "&Load";
					m_lnkSpotAction.Visible = true;
					mLinkClick = () => OnGenericSpotClick("Start-IndustrySpotLoad");

					m_lnkSpotActionAux.Text = "&Cancel reservation";
					m_lnkSpotActionAux.Visible = true;
					mLinkClickAux = () => OnGenericSpotClick("Clear-IndustrySpotReservation");
					break;

				case Tycoon.SpotStates.CAR_PARKED:
					m_lnkSpotAction.Text = "&Pickup car";
					m_lnkSpotAction.Visible = true;
					mLinkClick = () => OnGenericSpotClick("Remove-CarFromSpot");
					break;
			}
		}

		private void HideSpotLinks()
		{
			m_lnkSpotAction.Visible = false;
			m_lnkSpotActionAux.Visible = false;
		}

		private void m_cbSpot_SelectedIndexChanged(object sender, EventArgs e)
		{
			var item = m_cbSpot.SelectedItem as Tycoon.Spot;

			HideSpotLinks();

			if (item == null)
			{
				m_tbSpotInfo.Text = String.Empty;
				m_tbSpotState.Text = String.Empty;

				if (mSelectedSpot != null)
				{
					mSelectedSpot.PropertyChanged -= Spot_PropertyChanged;
					mSelectedSpot = null;
				}
			}
			else
			{
				RefreshSpotInfo(item);

				item.PropertyChanged += Spot_PropertyChanged;
				mSelectedSpot = item;
			}
		}

		private void Spot_PropertyChanged(object sender, PropertyChangedEventArgs e)
		{
			if (sender != mSelectedSpot)
				return;

			HideSpotLinks();
			RefreshSpotInfo(mSelectedSpot);
		}

		private async void RunFunc(Func<Task> a)
		{
			try
			{
				await a();
			}
			catch (Exception ex)
			{
				MessageBox.Show(this, "Error executing command: " + ex.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
			}
		}

		private void m_lnkSpotAction_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
		{
			RunFunc(mLinkClick);
		}

		private void m_lnkSpotActionAux_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
		{
			RunFunc(mLinkClickAux);
		}

		Task OnSetIndustrySpotReservedClick()
		{
			using var dialog = new SpotReservationInfoForm();
			dialog.ShowDialog();
			if (dialog.DialogResult != DialogResult.OK)
				return Task.CompletedTask;

			return mConsole.RequestAsync("Set-IndustrySpotReserved", mIndustry.Path, ((Tycoon.Spot)m_cbSpot.SelectedItem).Name, dialog.InfoText);
		}

		Task OnGenericSpotClick(string proc)
		{
			return mConsole.RequestAsync(proc, mIndustry.Path, ((Tycoon.Spot)m_cbSpot.SelectedItem).Name);
		}
	}
}
