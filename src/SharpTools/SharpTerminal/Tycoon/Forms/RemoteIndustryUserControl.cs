using SharpTerminal.Tycoon;
using SharpTerminal.Tycoon.Forms;
using System;
using System.ComponentModel;
using System.Runtime.Versioning;
using System.Threading.Tasks;
using System.Windows.Forms;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;
using static System.Windows.Forms.VisualStyles.VisualStyleElement.Window;

namespace SharpTerminal.Forms
{
	[SupportedOSPlatform("windows")]
	public partial class RemoteIndustryUserControl : UserControl
	{
		Tycoon.RemoteIndustry	mIndustry;
		Tycoon.Spot				mSelectedSpot;
		IConsole				mConsole;
		Func<Task>				mLinkClick;
		Func<Task>				mLinkClickAux;

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
				item.Tag = cargoInfo;

				m_lvProduction.Items.Add(item);

				cargoInfo.PropertyChanged += Industry_CargoInfoPropertyChanged;
			}

			foreach (ColumnHeader column in m_lvProduction.Columns)
			{
				column.Width = -2;
			}

			m_lvProduction.ResumeLayout();
		}

		private void Industry_CargoInfoPropertyChanged(object sender, PropertyChangedEventArgs e)
		{
			var cargoInfo = (CargoInfo)sender;
			
			foreach(ListViewItem item in m_lvProduction.Items)
			{
				if (item.Tag != cargoInfo)
					continue;
				
				item.SubItems[1].Text = cargoInfo.CurrentQuantity.ToString();
				item.SubItems[2].Text = cargoInfo.ReservedQuantity.ToString();
				item.SubItems[3].Text = (cargoInfo.CurrentQuantity + cargoInfo.ReservedQuantity).ToString();
				break;									
			}
		}

		private void Industry_PropertyChanged(object sender, PropertyChangedEventArgs e)
		{
			switch (e.PropertyName)
			{
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
			m_tbSpotCargoInformation.Text = spot.CargoInformation;

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
					mLinkClick = () => OnStartIndustrySpotLoadClick();

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

		Task OnStartIndustrySpotLoadClick()
		{
			CargoInfo cargo = null;

			//auto select if only one cargo is available, otherwise ask the user to select one
			if (m_lvProduction.Items.Count == 1)
			{
				cargo = (CargoInfo)m_lvProduction.Items[0].Tag;
			}
			else
			{
				var selectedItems = m_lvProduction.SelectedItems;
				if (selectedItems.Count == 0)
				{
					MessageBox.Show(this, "Please select a cargo to load.", "No cargo selected", MessageBoxButtons.OK, MessageBoxIcon.Warning);

					m_lvProduction.Focus();

					return Task.CompletedTask;
				}

				cargo = (CargoInfo)selectedItems[0].Tag;
			}					

			return mConsole.RequestAsync("Start-IndustrySpotLoad", mIndustry.Path, ((Tycoon.Spot)m_cbSpot.SelectedItem).Name, cargo.CargoName);
		}

		Task OnGenericSpotClick(string proc)
		{
			return mConsole.RequestAsync(proc, mIndustry.Path, ((Tycoon.Spot)m_cbSpot.SelectedItem).Name);
		}
	}
}
