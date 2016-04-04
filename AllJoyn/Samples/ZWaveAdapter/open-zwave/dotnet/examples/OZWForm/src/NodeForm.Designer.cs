namespace OZWForm
{
    partial class NodeForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.NodeLayoutPanel = new System.Windows.Forms.TableLayoutPanel();
            this.SuspendLayout();
            // 
            // NodeLayoutPanel
            // 
            this.NodeLayoutPanel.AutoScroll = true;
            this.NodeLayoutPanel.AutoSize = true;
            this.NodeLayoutPanel.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.NodeLayoutPanel.ColumnCount = 1;
            this.NodeLayoutPanel.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.NodeLayoutPanel.Location = new System.Drawing.Point(12, 12);
            this.NodeLayoutPanel.Name = "NodeLayoutPanel";
            this.NodeLayoutPanel.RowCount = 1;
            this.NodeLayoutPanel.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.NodeLayoutPanel.Size = new System.Drawing.Size(0, 0);
            this.NodeLayoutPanel.TabIndex = 0;
            // 
            // NodeForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoScroll = true;
            this.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.ClientSize = new System.Drawing.Size(526, 323);
            this.Controls.Add(this.NodeLayoutPanel);
            this.Name = "NodeForm";
            this.ShowInTaskbar = false;
            this.Text = "NodeForm";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TableLayoutPanel NodeLayoutPanel;
    }
}