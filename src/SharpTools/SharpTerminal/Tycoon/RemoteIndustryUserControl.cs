using SharpTerminal.Tycoon;
using System;
using System.ComponentModel;
using System.Net.Http.Headers;
using System.Runtime.Versioning;
using System.Windows.Forms;

namespace SharpTerminal.Forms
{
	[SupportedOSPlatform("windows")]
	public partial class RemoteIndustryUserControl : UserControl
	{
		Tycoon.RemoteIndustry	mIndustry;
		Tycoon.Spot				mSelectedSpot;
		IConsole				mConsole;

		string m_strLinkSpotActionCmd;

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

			m_tbCargo.Text = industry.CargoName;
			m_tbDailyRate.Text = industry.DailyRate.ToString();
			m_tbQuantity.Text = industry.CurrentQuantity.ToString();

			m_tbStatus.Text = industry.Producing ? "Next batch at " + industry.NextProductionAt : "Not producing";

			mIndustry.PropertyChanged += Industry_PropertyChanged;

			var spots = industry.GetSpots();
			foreach (var spot in spots)
			{
				var index = m_cbSpot.Items.Add(spot);
			}

			m_cbSpot.SelectedIndex = 0;
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

		private void RefreshSpotInfo(Spot spot)
		{
			m_tbSpotInfo.Text = spot.Information;
			m_tbSpotState.Text = spot.State.ToString();

			switch (spot.State)
			{
				case Tycoon.SpotStates.FREE:
					m_lnkSpotAction.Text = "Reserve";
					m_lnkSpotAction.Visible = true;
					m_strLinkSpotActionCmd = "Set-IndustrySpotReserved";
					break;

				case Tycoon.SpotStates.RESERVED:
					m_lnkSpotAction.Text = "Load";
					m_lnkSpotAction.Visible = true;
					m_strLinkSpotActionCmd = "Start-IndustrySpotLoad";

					m_lnkSpotActionAux.Text = "Cancel reservation";
					m_lnkSpotActionAux.Visible = true;
					m_strLinkSpotActionCmd = "Clear-IndustrySpotReservation";
					break;

				case Tycoon.SpotStates.CAR_PARKED:
					m_lnkSpotAction.Text = "Pickup car";
					m_lnkSpotAction.Visible = true;
					m_strLinkSpotActionCmd = "Set-IndustrySpotCarPicked";
					break;
			}
		}

		private void m_cbSpot_SelectedIndexChanged(object sender, EventArgs e)
		{
			var item = m_cbSpot.SelectedItem as Tycoon.Spot;

			m_lnkSpotAction.Visible = false;
			m_lnkSpotActionAux.Visible = false;

			m_strLinkSpotActionCmd = null;

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

			RefreshSpotInfo(mSelectedSpot);			
		}

		private async void m_lnkSpotAction_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
		{
			if (m_strLinkSpotActionCmd == null)
				return;

			try
			{
				await mConsole.RequestAsync(m_strLinkSpotActionCmd, mIndustry.Path, ((Tycoon.Spot)m_cbSpot.SelectedItem).Name, "Hello");
			}
			catch (Exception ex)
			{
				MessageBox.Show(this, "Error executing command: " + ex.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
			}
		}
	}
}
