﻿using System;
using System.IO;
using System.Windows.Forms;

namespace SharpEEPromViewer
{
    public partial class MainForm : Form
    {        
        public MainForm(string param = null)
        {
            InitializeComponent();

            DefaultIcons.LoadIcons(mImageList);
            mTreeView.ImageList = mImageList;

            if (param != null)
                this.LoadEEProm(param);
                
        }        

        private void btnExit_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void LoadEEProm(string filePath)
        {
            using var fileStream = new FileStream(filePath, FileMode.Open, FileAccess.Read);

            this.LoadEEProm(fileStream);
        }

        private static void ConfigureNode(TreeNode node, Lump lump)
        {
            node.ImageKey = DefaultIcons.FOLDER_ICON;
            node.SelectedImageKey = DefaultIcons.FOLDER_ICON;
            node.Tag = lump;
        }

        private static void FillTree_r(TreeNode parent, Lump lump)
        {
            if (lump.Children == null)
                return;

            foreach(var child in lump.Children)
            {
                var node = parent.Nodes.Add(child.Name);

                ConfigureNode(node, child);

                FillTree_r(node, child);
            }            
        }

        private void LoadEEProm(Stream stream)
        {
            using var reader = new System.IO.BinaryReader(stream, System.Text.Encoding.ASCII);

            var lump = Lump.Create(reader);
            
            if(lump is not RootLump)
            {
                MessageBox.Show("Error: file does not contains a valid header", "Error reading", MessageBoxButtons.OK, MessageBoxIcon.Error);

                return;
            }
            
            //so far it appears that the EEProm is fine, so clear and load
            this.Clear();

            mTreeView.Nodes.Add(lump.Name);

            var node = mTreeView.Nodes[0];

            ConfigureNode(node, lump);
            
            FillTree_r(node, lump);

            node.ExpandAll();
        }

        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            using (OpenFileDialog openFileDialog = new OpenFileDialog())
            {
                openFileDialog.Filter = "rom files (*.rom)|*.rom|All files (*.*)|*.*";
                openFileDialog.FilterIndex = 2;

                if (openFileDialog.ShowDialog() != DialogResult.OK)
                    return;

                //Get the path of specified file
                var filePath = openFileDialog.FileName;

                //Read the contents of the file into a stream
                using var fileStream = openFileDialog.OpenFile();

                this.LoadEEProm(fileStream);
                
                //fileStream.ReadByte
            }
        }

        private void Clear()
        {
            mTreeView.Nodes.Clear();
        }

        private void mTreeView_AfterSelect(object sender, TreeViewEventArgs e)
        {
            var selectedNode = mTreeView.SelectedNode;

            mSplitContainer.Panel2.Controls.Clear();

            if (selectedNode == null)            
                return;
                        
            var inspector = new ObjectInspectorUserControl((Lump)selectedNode.Tag);
            mSplitContainer.Panel2.Controls.Clear();

            inspector.Dock = DockStyle.Fill;
            mSplitContainer.Panel2.Controls.Add(inspector);
        }
    }
}